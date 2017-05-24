#include <stdio.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>

#include "cli_priv.h"
#include "oip_priv.h"
#include "plugin_priv.h"
#include "pipeline_priv.h"
#include "imgutil/imgutil.h"

#define CLI_GETOPT_OPTS "vi:"
#define SHELL_BUFFER_LEN 100

static pthread_t thread_cli_shell;

static void *cli_shell_run(void *args);
static void cli_shell_parse(char *str);

int cli_parse_opts(int argc, char **argv) {
	int ret;

	// Set the CLI option flags to zero initially.
	memset(&cli_opts, CLI_OPT_DISABLED, sizeof(cli_opts));

	while ((ret = getopt(argc, argv, CLI_GETOPT_OPTS)) != -1) {
		switch (ret) {
			case 'v':
				cli_opts.opt_verbose = CLI_OPT_ENABLED;
				break;
			case 'i':
				cli_opts.opt_image_path = calloc(strlen(optarg) + 1, sizeof(char));
				if (cli_opts.opt_image_path == NULL) {
					fprintf(stderr, "calloc(): Failed to allocate memory.\n");
					return 1;
				}
				strcpy(cli_opts.opt_image_path, optarg);
				break;
			case '?':
				if (optopt == 'i') {
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

void cli_opts_cleanup(void) {
	if (cli_opts.opt_image_path != NULL) {
		free(cli_opts.opt_image_path);
	}
}

pthread_t *cli_shell_init(void) {
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
		pthread_testcancel();
		if (fgets(shell_buff, SHELL_BUFFER_LEN, stdin) == NULL) {
			perror("fgets(): ");
			continue;
		}
		cli_shell_parse(shell_buff);
		memset(shell_buff, 0, SHELL_BUFFER_LEN*sizeof(char));
	}
	return NULL;
}

static void cli_shell_parse(char *str) {
	char keywords[40][40];
	int keyword_index = 0;
	int num_keywords = 0;
	int s = 0;

	// Parse command keywords into an array.
	memset(keywords, 0, 40*40*sizeof(char));
	for (int i = 0; i < strlen(str); i++) {
		if (str[i] == ' ' || str[i] == '\n') {
			memcpy(keywords[keyword_index], str + s, i - s);
			keyword_index++;
			s = i + 1;
		}
	}
	num_keywords = keyword_index;

	if (num_keywords == 0) {
		return;
	}

	// Execute the commands.
	if (strcmp(keywords[0], "exit") == 0) {
		printf("cli-shell: Exit!\n");
		oip_exit();
		return;
	} else if (strcmp(keywords[0], "plugload") == 0) {
		if (num_keywords == 3) {
			if (plugin_load(keywords[1], keywords[2]) != 0) {
				printf("cli-shell: Failed to load plugin.\n");
				return;
			}
		} else {
			printf("cli-shell: plugload missing arguments.\n");
			return;
		}
	} else if (strcmp(keywords[0], "plugprint") == 0) {
		print_plugin_config();
	} else if (strcmp(keywords[0], "feed") == 0) {
		if (num_keywords == 3) {
			printf("cli-shell: Loading %s.\n", keywords[1]);
			IMAGE *src = img_load(keywords[1]);
			if (src == NULL) {
				return;
			}
			IMAGE *result = img_alloc(0, 0);
			if (!result) {
				return;
			}
			if (pipeline_feed(src, result) != 0) {
				printf("cli-shell: Image processing failed.\n");
				return;
			}

			img_save(result, keywords[2]);

			img_free(src);
			img_free(result);
			return;
		} else {
			printf("cli-shell: feed missing arguments.\n");
			return;
		}
	}
}
