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

#define PRINT_IDENTIFIER "configloader"

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

#include "configloader_priv.h"
#include "headers/output.h"

#define CONFIG_DEFAULT_PATH "oip.conf"
#define CONFIG_BUF_LEN 100
#define CONFIG_NUM_VALID_PARAMS 2

static unsigned int config_num_params = 0;
static char **config = NULL;

static char *config_valid_params[CONFIG_NUM_VALID_PARAMS] = {
	"cache_root",
	"cache_default_max_files"
};

static int config_lineempty(const char *ln);
static int config_parse_line(const char *ln);
static int config_param_is_valid(const char *param);

static int config_lineempty(const char *ln) {
	/*
	*  Returns 1 if the supplied string only contains whitespace
	*  characters. Otherwise 0 is returned.
	*/
	char *tmp = (char*) ln;
	while (*tmp != '\0') {
		if (!isspace(*tmp)) {
			return 0;
		}
		tmp++;
	}
	return 1;
}

static int config_param_is_valid(const char *param) {
	/*
	*  Return 1 if 'param' is a valid configuration
	*  parameter and 0 otherwise.
	*/
	for (unsigned int i = 0; i < CONFIG_NUM_VALID_PARAMS; i++) {
		if (strcmp(param, config_valid_params[i]) == 0) {
			return 1;
		}
	}
	return 0;
}

static int config_parse_line(const char *ln) {
	/*
	*  Parse a the configuration file line 'ln'. The
	*  line doesn't have to be newline terminated.
	*  Returns 0 on success and 1 on failure.
	*/

	char **tmp_config = NULL;
	char *tmp_ln = NULL;
	char *token = NULL;

	if (strlen(ln) < 4) {
		return 1;
	}

	errno = 0;
	tmp_ln = calloc(strlen(ln) + 1, sizeof(*ln));
	if (!tmp_ln) {
		perror("configloader: calloc()");
		return 1;
	}
	strcpy(tmp_ln, ln);
	tmp_ln[strcspn(tmp_ln, "\n")] = '\0';

	// Extend the config array.
	errno = 0;
	tmp_config = realloc(config, (++config_num_params)*2*sizeof(*config));
	if (!tmp_config) {
		perror("configloader: realloc()");
		free(tmp_ln);
		return 1;
	}
	config = tmp_config;

	// Read the parameter, check validity and copy the data to the config array.
	token = strtok(tmp_ln, "=");
	if (!config_param_is_valid(token)) {
		printerr_va("Invalid configuration parameter: %s\n", token);
		errno = 0;
		config = realloc(config, (--config_num_params)*2*sizeof(*config));
		if (!config && config_num_params != 0) {
			perror("configloader: realloc()");
		}
		return 1;
	}

	errno = 0;
	config[config_num_params*2 - 2] = calloc(strlen(token) + 1, sizeof(**config));
	if (!config[config_num_params*2 - 2]) {
		free(tmp_ln);
		errno = 0;
		config = realloc(config, (--config_num_params)*2*sizeof(*config));
		if (!config && config_num_params != 0) {
			perror("configloader: calloc()");
		}
		return 1;
	}
	strcpy(config[config_num_params*2 - 2], token);

	token = strtok(NULL, "=");
	errno = 0;
	config[config_num_params*2 - 1] = calloc(strlen(token) + 1, sizeof(**config));
	if (!config[config_num_params*2 - 1]) {
		free(tmp_ln);
		free(config[config_num_params*2 - 2]);
		errno = 0;
		config = realloc(config, (--config_num_params)*2*sizeof(*config));
		if (!config && config_num_params != 0) {
			perror("configloader: calloc()");
		}
		return 1;
	}
	strcpy(config[config_num_params*2 - 1], token);
	free(tmp_ln);
	return 0;
}

char *config_get_str_param(const char *param) {
	/*
	*  Get the string value of 'param' or NULL
	*  if 'param' does not exist.
	*/
	for (unsigned int i = 0; i < config_num_params; i++) {
		if (strcmp(config[i*2], param) == 0) {
			return config[i*2 + 1];
		}
	}
	return NULL;
}

long int config_get_lint_param(const char *param) {
	/*
	*  Get the long int value of 'param' or 0 if
	*  'param' does not exist.
	*/
	long int val = 0;
	char *str_val = NULL;
	str_val = config_get_str_param(param);
	if (!str_val) {
		return 0;
	}

	errno = 0;
	val = strtol(str_val, NULL, 10);
	if (errno != 0) {
		perror("configloader: strtol()");
	}
	return val;
}

int config_load(char *cfpath) {
	/*
	*  Load configuration from file. Returns 0 on success
	*  and 1 on failure.
	*/
	FILE *conf;
	char linebuf[CONFIG_BUF_LEN] = { '\0' };
	char *ret = NULL;

	errno = 0;
	if (!cfpath) {
		printverb_va("Loading configuration from file '%s'.\n", CONFIG_DEFAULT_PATH);
		conf = fopen(CONFIG_DEFAULT_PATH, "r");
	} else {
		printverb_va("Loading configuration from file '%s'.\n", cfpath);
		conf = fopen(cfpath, "r");
	}

	if (!conf) {
		perror("config: fopen()");
		return 1;
	}

	while (!feof(conf)) {
		if ((ret = fgets(linebuf, CONFIG_BUF_LEN, conf))) {
			if (!config_lineempty(linebuf)) {
				config_parse_line(linebuf);
			}
		}
		if (ferror(conf)) {
			perror("config: fgets()");
			return 1;
		}
	}
	return 0;
}

void config_cleanup(void) {
	printverb("Configuration cleanup.\n");
	if (config) {
		for (unsigned int i = 0; i < config_num_params*2; i++) {
			free(config[i]);
		}
		free(config);
	}
}
