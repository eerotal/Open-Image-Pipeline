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

#define CLI_GETOPT_OPTS "vp"
#define SHELL_BUFFER_LEN 100

#define NUM_CLI_CMD_PROTOS 9
#define NUM_CLI_CMD_MAX_KEYWORDS 10

static pthread_t thread_cli_shell;
static JOB **cli_shell_jobs = NULL;
static unsigned int cli_shell_jobs_count = 0;

static char *cli_cmd_prototypes[NUM_CLI_CMD_PROTOS][NUM_CLI_CMD_MAX_KEYWORDS] = {
	{"plugin", "load", "%s", "%s"},
	{"plugin", "list"},
	{"plugin", "set-arg", "%s", "%s", "%s"},
	{"job", "create", "%s"},
	{"job", "feed", "%s"},
	{"job", "delete", "%s"},
	{"job", "save", "%s"},
	{"job", "list"},
	{"exit"}
};

static void cli_shell_cleanup(void *arg);
static void *cli_shell_run(void *args);
static int cli_shell_parse(char *str);
static int cli_shell_prototype_match(char **keywords, unsigned int num_keywords);
static void cli_shell_execute(unsigned int proto, char **keywords, unsigned int num_keywords);
static int cli_shell_jobs_shrink(void);
static int cli_shell_job_add(JOB *job);
static int cli_shell_job_delete(unsigned int index);

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
				printf("cli: Cache preservation enabled!");
				cli_opts.opt_preserve_cache = CLI_OPT_ENABLED;
				break;
			case '?':
				if (isprint(optopt)) {
					fprintf(stderr, "cli: Unknown option -%c.\n", optopt);
				} else {
					fprintf(stderr, "cli: Unknown option character. 0x%x.", optopt);
				}
				return 1;
			default:
				break;
		}
	}
	for (int i = optind; i < argc; i++) {
		printf("cli: Non-option argument %s discarded.\n", argv[i]);
	}
	return 0;
}

const struct CLI_OPTS *cli_get_opts(void) {
	return &cli_opts;
}

static void cli_shell_cleanup(void *arg) {
	// Free the jobs array.
	printf("cli-shell: CLI shell cleanup.\n");
	if (cli_shell_jobs != NULL) {
		for (unsigned int i = 0; i < cli_shell_jobs_count; i++) {
			if (cli_shell_jobs[i] != NULL) {
				job_destroy(cli_shell_jobs[i]);
			}
		}
		free(cli_shell_jobs);
	}
}

pthread_t *cli_shell_init(void) {
	errno = 0;
	if (pthread_create(&thread_cli_shell, NULL, &cli_shell_run, NULL) != 0) {
		perror("cli-shell: pthread_create(): ");
		return NULL;
	}
	return &thread_cli_shell;
}

static void *cli_shell_run(void *args) {
	char shell_buff[SHELL_BUFFER_LEN] = { '\0' };

	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
	pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);
	pthread_cleanup_push(cli_shell_cleanup, NULL);

	printf("cli-shell: Thread started. Shell buffer: %i b.\n", SHELL_BUFFER_LEN);
	for (;;) {
		errno = 0;
		while (fgets(shell_buff, SHELL_BUFFER_LEN, stdin) == NULL) {
			pthread_testcancel();
			if (errno != 0) {
				perror("cli-shell: fgets(): ");
			}
		}
		if (cli_shell_parse(shell_buff) != 0) {
			printf("cli-shell: Command parsing failed.\n");
		}
		memset(shell_buff, 0, SHELL_BUFFER_LEN*sizeof(char));
	}

	pthread_cleanup_pop(1);
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

static int cli_shell_jobs_shrink(void) {
	/*
	*  Shrink the cli_shell_jobs array removing any
	*  NULL pointers from it.
	*/
	unsigned int new_jobs_count = 0;
	JOB **new_jobs = NULL;
	JOB **new_jobs_tmp = NULL;

	for (unsigned int i = 0; i < cli_shell_jobs_count; i++) {
		if (cli_shell_jobs[i] != NULL) {
			new_jobs_count++;
			errno = 0;
			new_jobs_tmp = realloc(new_jobs, new_jobs_count*sizeof(JOB*));
			if (new_jobs_tmp == NULL) {
				perror("cli-shell: realloc(): ");
				free(new_jobs_tmp);
				return 1;
			}
			new_jobs = new_jobs_tmp;
			new_jobs[new_jobs_count - 1] = cli_shell_jobs[i];
		}
	}
	free(cli_shell_jobs);
	cli_shell_jobs = new_jobs;
	cli_shell_jobs_count = new_jobs_count;
	return 0;
}

static int cli_shell_job_delete(unsigned int index) {
	/*
	*  Delete a job from the cli_shell_jobs array and
	*  shrink the array afterwards.
	*/
	if (index >= cli_shell_jobs_count) {
		return 1;
	}
	job_destroy(cli_shell_jobs[index]);
	cli_shell_jobs[index] = NULL;
	if (cli_shell_jobs_shrink() != 0) {
		return 1;
	}
	return 0;
}

static int cli_shell_job_add(JOB *job) {
	JOB **tmp_jobs = NULL;

	if (job == NULL) {
		return 1;
	}

	cli_shell_jobs_count++;

	// Extend the cli_shell_jobs array.
	errno = 0;
	tmp_jobs = realloc(cli_shell_jobs, cli_shell_jobs_count*sizeof(JOB*));
	if (tmp_jobs == NULL) {
		cli_shell_jobs_count--;
		perror("cli-shell: realloc(): ");
		return 1;
	}
	cli_shell_jobs = tmp_jobs;

	// Add the new pointer to the array.
	cli_shell_jobs[cli_shell_jobs_count - 1] = job;

	return 0;
}

static void cli_shell_execute(unsigned int proto, char **keywords, unsigned int num_keywords) {
	/*
	*  Execute the command in keywords that matches the command
	*  prototype proto.
	*/
	unsigned int tmp_index = 0;
	if (proto >= NUM_CLI_CMD_PROTOS) {
		return;
	}

	switch (proto) {
		case 0: ; // plugin load %s %s
			if (plugin_load(keywords[2], keywords[3]) != 0) {
				printf("cli-shell: Failed to load plugin.\n");
			}
			break;
		case 1: ; // plugin list
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
		case 3: ; // job create %s %s
			JOB *tmp_job = job_create(keywords[2]);
			if (tmp_job == NULL) {
				printf("cli-shell: Failed to create job.\n");
				break;
			}
			if (cli_shell_job_add(tmp_job) != 0) {
				printf("cli-shell: Failed to append job to jobs array.\n");
			}
			break;
		case 4: ; // job feed %s
			if (!isdigit(keywords[2][0])) {
				printf("cli-shell: Invalid job index.\n");
				break;
			}

			tmp_index = strtol(keywords[2], NULL, 10);
			if (tmp_index < cli_shell_jobs_count) {
				if (pipeline_feed(cli_shell_jobs[tmp_index]) != 0) {
					printf("cli-shell: Image processing failed.\n");
				}
			} else {
				printf("cli-shell: Job index out of range.\n");
			}
			break;
		case 5: ; // job delete %s
			if (!isdigit(keywords[2][0])) {
				printf("cli-shell: Invalid job index.\n");
				break;
			}

			tmp_index = strtol(keywords[2], NULL, 10);
			if (tmp_index < cli_shell_jobs_count) {
				if (cli_shell_job_delete(tmp_index) != 0) {
					printf("cli-shell: Job deletion failed.\n");
				}
			} else {
				printf("cli-shell: Job index out of range.\n");
			}
			break;
		case 6: ; // job save %s
			if (!isdigit(keywords[2][0])) {
				printf("cli-shell: Invalid job index.\n");
				break;
			}

			tmp_index = strtol(keywords[2], NULL, 10);
			if (tmp_index < cli_shell_jobs_count) {
				if (job_save_result(cli_shell_jobs[tmp_index], keywords[3]) != 0) {
					printf("cli-shell: Failed to save image.\n");
				}
			} else {
				printf("cli-shell: Job index out of range.\n");
			}
			break;
		case 7: ; // job list
			for (unsigned int i = 0; i < cli_shell_jobs_count; i++) {
				job_print(cli_shell_jobs[i]);
			}
			break;
		case 8: ; // exit
			oip_exit();
			break;
		default:
			break;
	}
}

static int cli_shell_parse(char *str) {
	/*
	*  Parse the newline terminated command from
	*  string and execute it afterwards. Returns 0
	*  on success and 1 on failure.
	*/
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
				perror("cli-shell: realloc(): ");
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
				perror("cli-shell: calloc(): ");
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
			perror("cli-shell: malloc(): ");
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
