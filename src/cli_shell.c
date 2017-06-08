/*
*
*  Copyright 2017 Eero Talus
*
*  This file is part of Open Image Pipeline.
*
*  Open Image Pipeline is free software: you can redistribute it and/or modify
*  it under the terms of the GNU General Public License as published by
*  the Free Software Foundation, either version 3 of the License, or
*  (at your option) any later version.
*
*  Open Image Pipeline is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License
*  along with Open Image Pipeline.  If not, see <http://www.gnu.org/licenses/>.
*
*/

#define PRINT_IDENTIFIER "cli-shell"

#include <stdio.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>

#include "headers/output.h"
#include "cli_shell_priv.h"
#include "oip_priv.h"
#include "plugin_priv.h"
#include "pipeline_priv.h"
#include "ptrarray_priv.h"
#include "jobmanager_priv.h"
#include "build_priv.h"

#define SHELL_BUFFER_LEN 100
#define NUM_CLI_CMD_PROTOS 12
#define NUM_CLI_CMD_MAX_KEYWORDS 10

PTRARRAY_TYPE_DEF(char);

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
	{"cache", "dump", "all"},
	{"cache", "file", "delete", "%s", "%s"},
	{"help"},
	{"exit"}
};

static char *cli_cmd_help[NUM_CLI_CMD_PROTOS] = {
	"plugin load <directory> <plugin name>  -------  Load the plugin <plugin name> from <directory>.",
	"plugin list  ---------------------------------  List all loaded plugins.",
	"plugin set-arg <plugin index> <arg> <val>  ---  Set the argument <arg> to <val> for plugin <plugin index>.",
	"job create <filepath>  -----------------------  Create a job for <filepath>.",
	"job feed <ID>  -------------------------------  Feed the job with the ID <ID> to the pipeline.",
	"job delete <ID>  -----------------------------  Delete the job with the ID <ID>.",
	"job save <ID>  -------------------------------  Save the result image of the job with the ID <ID>.",
	"job list  ------------------------------------  List all jobs.",
	"cache dump all  ------------------------------  Dump information about existing caches to STDOUT.",
	"cache file delete <cache> <fname> ------------  Delete the file <fname> from <cache>.",
	"help  ----------------------------------------  Print this help.",
	"exit  ----------------------------------------  Exit the program."
};

static void cli_shell_cleanup(void *arg);
static void *cli_shell_run(void *args);
static int cli_shell_parse(char *str);
static int cli_shell_prototype_match(const PTRARRAY_TYPE(char) *keywords);
static void cli_shell_execute(const size_t proto,
		const PTRARRAY_TYPE(char) *keywords);
static void cli_shell_print_help(void);

static void cli_shell_cleanup(void *arg) {
	// Free the jobs array.
	(void) arg; // Suppress warning about unused parameter.
	printverb("CLI shell cleanup.\n");
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
		printerrno("cli-shell: pthread_create()");
		return NULL;
	}

	printf("\nOpen Image Pipeline Copyright (C) 2017 Eero Talus\n");
	printf("This program is licensed under the GNU General Public License\n");
	printf("version 3 and comes with ABSOLUTELY NO WARRANTY. This program is\n");
	printf("also free software. See the file LICENSE.txt for more details\n");
	printf("about the license and the file README.md for general information.\n\n");

	build_print_version_info();

	return &thread_cli_shell;
}

static void *cli_shell_run(void *args) {
	(void) args; // Suppress warning about unused parameter.
	char shell_buff[SHELL_BUFFER_LEN] = { '\0' };

	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
	pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);
	pthread_cleanup_push(cli_shell_cleanup, NULL);

	printverb_va("Thread started. Shell buffer: %i b.\n", SHELL_BUFFER_LEN);
	for (;;) {
		errno = 0;
		while (fgets(shell_buff, SHELL_BUFFER_LEN, stdin) == NULL) {
			pthread_testcancel();
			if (errno != 0) {
				printerrno("cli-shell: fgets()");
			}
		}
		if (cli_shell_parse(shell_buff) != 0) {
			printerr("Command parsing failed.\n");
		}
		memset(shell_buff, 0, SHELL_BUFFER_LEN*sizeof(char));
	}

	pthread_cleanup_pop(1);
	return NULL;
}

static int cli_shell_prototype_match(const PTRARRAY_TYPE(char) *keywords) {
	/*
	*  Return the CMD prototype index that matched the supplied CMD.
	*  If no match was found this function returns a negative number.
	*/

	int ret = -1;
	if (keywords->ptrc > NUM_CLI_CMD_MAX_KEYWORDS) {
		printerr("Too many keywords.");
		return -3;
	}

	for (size_t proto = 0; proto < NUM_CLI_CMD_PROTOS; proto++) {
		for (size_t k = 0; k < NUM_CLI_CMD_MAX_KEYWORDS; k++) {
			if (cli_cmd_prototypes[proto][k] == NULL) {
				// Every proto keyword that wasn't NULL matched.
				return ret;
			} else if (k >= keywords->ptrc) {
				// Missing arguments.
				ret = -2;
				break;
			} else {
				if (strcmp(cli_cmd_prototypes[proto][k], "%s") == 0) {
					// Text wildcard in proto.
					ret = proto;
					continue;
				}
				if (strcmp(cli_cmd_prototypes[proto][k], keywords->ptrs[k]) != 0) {
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

static void cli_shell_print_help(void) {
	printf("Open Image Pipeline CLI Shell interface help.\n");
	for (unsigned int i = 0; i < NUM_CLI_CMD_PROTOS; i++) {
		printf("  %s\n", cli_cmd_help[i]);
	}
}

static void cli_shell_execute(const size_t proto,
		const PTRARRAY_TYPE(char) *keywords) {
	/*
	*  Execute the command in keywords that matches the command
	*  prototype proto.
	*/
	JOB *tmp_job = NULL;

	if (proto >= NUM_CLI_CMD_PROTOS) {
		return;
	}

	switch (proto) {
		case 0: ; // plugin load %s %s
			if (plugin_load(keywords->ptrs[2], keywords->ptrs[3]) != 0) {
				printerr("Failed to load plugin.\n");
			}
			break;
		case 1: ; // plugin list
			print_plugin_config();
			break;
		case 2: ; // plugin set-arg %s %s %s
			size_t index = 0;
			for (size_t i = 0; i < strlen(keywords->ptrs[2]); i++) {
				if (!isdigit(keywords->ptrs[2][i])) {
					printerr("Invalid plugin index.\n");
					break;
				}
			}
			index = strtol(keywords->ptrs[2], NULL, 10);
			plugin_set_arg(index, keywords->ptrs[3], keywords->ptrs[4]);
			break;
		case 3: ; // job create %s %s
			tmp_job = job_create(keywords->ptrs[2]);
			if (!tmp_job) {
				printerr("Failed to create job.\n");
				break;
			}
			if (jobmanager_reg_job(tmp_job) != 0) {
				printerr("Failed to register the JOB with the jobmanager.\n");
			}
			break;
		case 4: ; // job feed %s
			tmp_job = jobmanager_get_job_by_id(keywords->ptrs[2]);
			if (!tmp_job) {
				break;
			}
			if (pipeline_feed(tmp_job) != 0) {
				printerr("Image processing failed.\n");
			}
			break;
		case 5: ; // job delete %s
			tmp_job = jobmanager_get_job_by_id(keywords->ptrs[2]);
			if (!tmp_job) {
				break;
			}
			if (jobmanager_unreg_job(tmp_job, 1) != 0) {
				printerr("Job deletion failed.\n");
			}
			break;
		case 6: ; // job save %s
			tmp_job = jobmanager_get_job_by_id(keywords->ptrs[2]);
			if (!tmp_job) {
				break;
			}
			if (job_save_result(tmp_job, keywords->ptrs[3]) != 0) {
				printerr("Failed to save image.\n");
			}
			break;
		case 7: ; // job list
			jobmanager_list();
			break;
		case 8: ; // cache dump
			cache_dump_all();
			break;
		case 9: ; // cache file delete %s %s
			CACHE *tmp_cache = NULL;
			tmp_cache = cache_get_by_name(keywords->ptrs[3]);
			if (tmp_cache == NULL) {
				printerr_va("Failed to find cache %s.\n", keywords->ptrs[3]);
				break;
			}

			if (cache_delete_file(tmp_cache, keywords->ptrs[4]) != 0) {
				printerr("Failed to delete cache file.\n");
			}
			break;
		case 10: ; // help
			cli_shell_print_help();
			break;
		case 11: ; // exit
			oip_exit();
			break;
		default:
			break;
	}
}

static int cli_shell_parse(char *str) {
	/*
	*  Parse and execute a CLI shell command from str.
	*  Returns 0 on success and 1 on failure.
	*/

	PTRARRAY_TYPE(char) *keywords = NULL;
	char *token = NULL;
	char *tmp_str = NULL;
	int proto = -1;

	tmp_str = calloc(strlen(str) + 1, sizeof(*str));
	if (!tmp_str) {
		return 1;
	}
	strcpy(tmp_str, str);
	tmp_str[strcspn(tmp_str, "\n")] = '\0';

	keywords = (PTRARRAY_TYPE(char)*) ptrarray_create(&free);
	if (!keywords) {
		free(tmp_str);
		return 1;
	}

	token = strtok(tmp_str, " ");
	while (token != NULL) {
		if (!ptrarray_put_data((PTRARRAY_TYPE(void)*) keywords, token,
				(strlen(token) + 1)*sizeof(*token))) {

			ptrarray_free_ptrs((PTRARRAY_TYPE(void)*) keywords);
			ptrarray_free((PTRARRAY_TYPE(void)*) keywords);
			free(tmp_str);
			return 1;
		}
		token = strtok(NULL, " ");
	}

	proto = cli_shell_prototype_match(keywords);
	if (proto >= 0) {
		cli_shell_execute(proto, keywords);
	} else {
		printerr_va("Invalid command: %s\n", tmp_str);
	}

	ptrarray_free_ptrs((PTRARRAY_TYPE(void)*) keywords);
	ptrarray_free((PTRARRAY_TYPE(void)*) keywords);
	free(tmp_str);

	return 0;
}
