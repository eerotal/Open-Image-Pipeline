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

#include "oipcore/abi/output.h"
#include "configloader_priv.h"

#define CONFIG_DEFAULT_PATH "oip.conf"
#define CONFIG_NUM_VALID_PARAMS 2

static PTRARRAY_TYPE(DP_VAR) *config = NULL;

static const char *config_valid_params[CONFIG_NUM_VALID_PARAMS] = {
	"cache_root",
	"cache_default_max_files"
};

PTRARRAY_TYPE(DP_VAR) *config_get(void) {
	/*
	*  Return a pointer to the config PTRARRAY.
	*/
	return config;
}

long config_get_lint_param(const char *param, size_t index) {
	DP_VAR *tmp = dp_get_var(config, param);
	if (tmp) {
		return dp_var_lint_value(tmp, index);
	}
	return 0;
}

char *config_get_str_param(const char *param, size_t index) {
	DP_VAR *tmp = dp_get_var(config, param);
	if (tmp) {
		return dp_var_str_value(tmp, index);
	}
	return NULL;
}

int config_load(char *cfpath) {
	/*
	*  Load configuration from file. Returns 0 on success
	*  and 1 on failure.
	*/
	FILE *conf = NULL;
	char *buf = NULL;
	long int conf_size = 0;

	errno = 0;
	if (!cfpath) {
		printverb_va("Loading configuration from "\
				"file '%s'.\n", CONFIG_DEFAULT_PATH);
		conf = fopen(CONFIG_DEFAULT_PATH, "r");
	} else {
		printverb_va("Loading configuration from "\
				"file '%s'.\n", cfpath);
		conf = fopen(cfpath, "r");
	}

	if (!conf) {
		printerrno("fopen()");
		return 1;
	}

	// Get the total size of the config file.
	errno = 0;
	if (fseek(conf, 0, SEEK_END) != 0) {
		printerrno("fseek()");
		fclose(conf);
		return 1;
	}
	errno = 0;
	conf_size = ftell(conf);
	if (conf_size == -1) {
		printerrno("ftell()");
		fclose(conf);
		return 1;
	}
	printverb_va("Allocating %li bytes for the "\
			"config buffer.\n", conf_size);

	errno = 0;
	if (fseek(conf, 0, SEEK_SET) != 0) {
		printerrno("fseek()");
		fclose(conf);
		return 1;
	}

	errno = 0;
	buf = malloc(conf_size + 1);
	if (!buf) {
		printerrno("malloc()");
		fclose(conf);
		return 1;
	}
	memset(buf, 0, conf_size + 1);

	// Read the config file contents int buf.
	errno = 0;
	fread(buf, sizeof(char), conf_size, conf);
	fclose(conf);
	if (ferror(conf) != 0) {
		printerrno("fread()");
		free(buf);
		return 1;
	}

	config = dp_parse_multiline(buf, config_valid_params,
					CONFIG_NUM_VALID_PARAMS);
	free(buf);
	if (!config) {
		printerr("Failed to parse configuration file.\n");
		return 1;
	}
	return 0;
}

void config_cleanup(void) {
	printverb("Configuration cleanup.\n");
	if (config) {
		ptrarray_free_ptrs((PTRARRAY_TYPE(void)*) config);
		ptrarray_free((PTRARRAY_TYPE(void)*) config);
	}
}
