#include <stdio.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>

#include "cli_priv.h"
#include "oip_priv.h"
#include "plugin_priv.h"
#include "pipeline_priv.h"
#include "imgutil/imgutil.h"


static struct CLI_OPTS cli_opts;

#define CLI_GETOPT_OPTS "v:o:"
#define SHELL_BUFFER_LEN 100

#define NUM_CLI_CMD_PROTOS 5
#define NUM_CLI_CMD_MAX_KEYWORDS 10

static pthread_t thread_cli_shell;

static char *cli_cmd_prototypes[NUM_CLI_CMD_PROTOS][NUM_CLI_CMD_MAX_KEYWORDS] = {
	{"plugin", "load", "%s", "%s"},
	{"plugin", "print"},
	{"plugin", "set-arg", "%s", "%s", "%s"},
	{"feed", "%s", "%s"},
	{"exit"}
};

static void *cli_shell_run(void *args);
static int cli_shell_parse(char *str);
static int cli_shell_prototype_match(char **keywords, unsigned int num_keywords);
static int cli_shell_execute(unsigned int proto, char **keywords, unsigned int num_keywords);

int cli_parse_opts(int argc, char **argv) {
	int ret;

	// Set the CLI option flags to zero initially.
	memset(&cli_opts, CLI_OPT_DISABLED, sizeof(cli_opts));

	while ((ret = getopt(argc, argv, CLI_GETOPT_OPTS)) != -1) {
		switch (ret) {
			case 'v':
				cli_opts.opt_verbose = CLI_OPT_ENABLED;
				break;
			case 'p':
				printf("cli: Cache preserve enabled!");
				cli_opts.opt_preserve_cache = CLI_OPT_ENABLED;
				break;
			case '?':
				if (optopt == 'i' || optopt == 'o') {
					fprintf(stderr, "Option -%c requires an argument.\n", optopt);
				} else if (isprint(optopt)) {
					fprintf(stderr, "Unknown option -%c.\n", optopt);
				} else {
					fprintf(stderr, "Unknown option character. 0x%x.", optopt);
				}
				return 1;
			default:
				break;
		}
	}
	for (int i = optind; i < argc; i++) {
		printf("Non-option argument %s discarded.\n", argv[i]);
	}
	return 0;
}

const struct CLI_OPTS *cli_get_opts(void) {
	return &cli_opts;
}

void cli_opts_cleanup(void) {

}

pthread_t *cli_shell_init(void) {
	errno = 0;
	if (pthread_create(&thread_cli_shell, NULL, &cli_shell_run, NULL) != 0) {
		perror("pthread_create(): ");
		return NULL;
	}
	return &thread_cli_shell;
}

static void *cli_shell_run(void *args) {
	char shell_buff[SHELL_BUFFER_LEN] = { '\0' };

	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
	pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);

	printf("cli-shell: Thread started. Shell buffer: %i b.\n", SHELL_BUFFER_LEN);
	for (;;) {
		printf(">> ");
		errno = 0;
		while (fgets(shell_buff, SHELL_BUFFER_LEN, stdin) == NULL) {
			pthread_testcancel();
			if (errno != 0) {
				perror("fgets(): ");
			}
		}
		if (cli_shell_parse(shell_buff) != 0) {
			printf("cli-shell: Command parsing failed.\n");
		}
		memset(shell_buff, 0, SHELL_BUFFER_LEN*sizeof(char));
	}
	return NULL;
}

static int cli_shell_prototype_match(char **keywords, unsigned int num_keywords) {
	/*
	*  Return the CMD prototype index that matched the supplied CMD.
	*  If no match was found this function returns a negative number.
	*/

	int ret = -1;
	if (num_keywords > NUM_CLI_CMD_MAX_KEYWORDS) {
		printf("cli-shell: Too many keywords.");
		return -3;
	}

	for (int proto = 0; proto < NUM_CLI_CMD_PROTOS; proto++) {
		for (int k = 0; k < NUM_CLI_CMD_MAX_KEYWORDS; k++) {
			if (cli_cmd_prototypes[proto][k] == NULL) {
				// Every proto keyword that wasn't NULL matched.
				return ret;
			} else if (k >= num_keywords) {
				// Missing arguments.
				ret = -2;
				break;
			} else {
				if (strcmp(cli_cmd_prototypes[proto][k], "%s") == 0) {
					// Text wildcard in proto.
					ret = proto;
					continue;
				}
				if (strcmp(cli_cmd_prototypes[proto][k], keywords[k]) != 0) {
					// Proto keyword doesn't match.
					ret = -1;
					break;
				}
				ret = proto;
			}
		}
		if (ret >= 0) {
			// Every proto key matched.
			return ret;
		}
	}
	return ret;
}

static int cli_shell_execute(unsigned int proto, char **keywords, unsigned int num_keywords) {
	if (proto >= NUM_CLI_CMD_PROTOS) {
		return 1;
	}

	switch (proto) {
		case 0: ; // plugin load %s %s
			if (plugin_load(keywords[2], keywords[3]) != 0) {
				printf("cli-shell: Failed to load plugin.\n");
			}
			break;
		case 1: ; // plugin print
			print_plugin_config();
			break;
		case 2: ; // plugin set-arg %s %s %s
			unsigned int index = 0;
			for (int i = 0; i < strlen(keywords[2]); i++) {
				if (!isdigit(keywords[2][i])) {
					printf("cli-shell: Invalid plugin index.\n");
					break;
				}
			}
			index = strtol(keywords[2], NULL, 10);
			plugin_set_arg(index, keywords[3], keywords[4]);
			break;
		case 3: ; // feed %s %s
			IMAGE *src = img_load(keywords[1]);
			if (src == NULL) {
				break;
			}
			IMAGE *result = img_alloc(0, 0);
			if (!result) {
				break;
			}

			// Generate a cache ID.
			char *cache_id = pipeline_gen_new_cache_id();
			if (cache_id == NULL) {
				img_free(src);
				img_free(result);
				break;
			}

			if (pipeline_feed(src, result, cache_id) != 0) {
				printf("cli-shell: Image processing failed.\n");
				img_free(src);
				img_free(result);
				free(cache_id);
				break;
			}

			img_save(result, keywords[2]);

			img_free(src);
			img_free(result);
			free(cache_id);
			break;
		case 4: ; // exit
			oip_exit();
			break;
	}
	return 0;
}

static int cli_shell_parse(char *str) {
	char **keywords = NULL;
	unsigned int c_keywords_len = 0;
	unsigned int num_keywords = 0;
	unsigned int s = 0;

	// Parse command keywords into an array.
	for (int i = 0; i < strlen(str); i++) {
		if (str[i] == ' ' || str[i] == '\n') {
			// Extend keywords array.
			c_keywords_len++;
			errno = 0;
			char **tmp_keywords = realloc(keywords, c_keywords_len*sizeof(char*));
			if (tmp_keywords == NULL) {
				/*
				*  Realloc failed so decrement length back to
				*  original and free the keywords array.
				*/
				perror("realloc(): ");
				c_keywords_len--;
				for (int k = 0; k < c_keywords_len; k++) {
					free(keywords[k]);
				}
				free(keywords);
				return 1;
			}
			keywords = tmp_keywords;

			// Allocate memory for the string to be copied.
			errno = 0;
			keywords[c_keywords_len - 1] = calloc(i - s, sizeof(char));
			if (keywords[c_keywords_len - 1] == NULL) {
				// Free the keywords array.
				perror("calloc(): ");
				for (int k = 0; k < c_keywords_len; k++) {
					free(keywords[k]);
				}
				free(keywords);
				return 1;
			}

			// Copy the the original string into the keywords array.
			memcpy(keywords[c_keywords_len - 1], str + s, i - s);
			s = i + 1;
		}
	}
	num_keywords = c_keywords_len;

	// Execute command based on matched prototype.
	int match = cli_shell_prototype_match(keywords, num_keywords);
	if (match >= 0) {
		cli_shell_execute(match, keywords, num_keywords);
	} else {
		/*
		*  Strip newline from input string and print
		*  "Invalid command".
		*/
		errno = 0;
		char *tmp_str = malloc(strlen(str)*sizeof(char));
		if (tmp_str == NULL) {
			perror("malloc(): ");
			return 1;
		}
		memcpy(tmp_str, str, strlen(str)*sizeof(char));
		tmp_str[strcspn(tmp_str, "\n")] = '\0';

		printf("cli-shell: Invalid command %s.\n", tmp_str);
		free(tmp_str);
	}

	// Cleanup
	for (int k = 0; k < c_keywords_len; k++) {
		free(keywords[k]);
	}
	free(keywords);
	return 0;
}
