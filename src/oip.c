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

#define PRINT_IDENTIFIER "oip"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <pthread.h>

#include "headers/output.h"
#include "plugin_priv.h"
#include "pipeline_priv.h"
#include "cli_priv.h"
#include "imgutil/imgutil.h"
#include "configloader_priv.h"

static pthread_t *thread_cli_shell;
static int exit_queued = 0;

static void oip_cleanup(void);

static void oip_cleanup(void) {
	// Cancel the CLI shell thread.
	if (thread_cli_shell) {
		if (pthread_cancel(*thread_cli_shell) != 0) {
			perror("oip: pthread_cancel()");
		}

		if (pthread_join(*thread_cli_shell, NULL) != 0) {
			perror("oip: pthread_join()");
		}
	}

	// Run cleanup functions.
	plugins_cleanup();
	config_cleanup();
}

void oip_exit(void) {
	exit_queued = 1;
}

int main(int argc, char **argv) {
	atexit(&oip_cleanup);

	// Read CLI options.
	if (cli_parse_opts(argc, argv) != 0) {
		printf("oip: CLI argument parsing failed.\n");
		return 1;
	}

	// Enable/disable verbose printing.
	if (cli_get_opts()->opt_verbose) {
		print_verbose_on();
	} else {
		print_verbose_off();
	}

	// Load configuration from file.
	if (config_load(cli_get_opts()->opt_config_file) != 0) {
		printerr("Failed to load configuration file.\n");
		return 1;
	}

	// Setup the plugin system.
	if (plugins_setup() != 0) {
		printerr("Failed to setup the plugin system.\n");
		return 1;
	}

	// Init CLI shell.
	thread_cli_shell = cli_shell_init();
	if (thread_cli_shell == NULL) {
		return 1;
	}

	while (!exit_queued) {}

	return 0;
}
