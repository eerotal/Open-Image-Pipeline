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

#define PRINT_IDENTIFIER "cli"

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>

#include "oip/abi/output.h"
#include "cli_priv.h"

#define CLI_GETOPT_OPTS "vpc:"

static struct CLI_OPTS cli_opts;

int cli_parse_opts(int argc, char **argv) {
	int ret;

	// Init CLI options to 0.
	memset(&cli_opts, CLI_OPT_DISABLED, sizeof(cli_opts));

	while ((ret = getopt(argc, argv, CLI_GETOPT_OPTS)) != -1) {
		switch (ret) {
			case 'v':
				cli_opts.opt_verbose = CLI_OPT_ENABLED;
				break;
			case 'p':
				cli_opts.opt_preserve_cache = CLI_OPT_ENABLED;
				break;
			case 'c':
				cli_opts.opt_config_file = optarg;
				break;
			case '?':
				if (isprint(optopt)) {
					printerr_va("Unknown option -%c.\n", optopt);
				} else {
					printerr_va("Unknown option character. 0x%x.", optopt);
				}
				return 1;
			default:
				break;
		}
	}
	for (int i = optind; i < argc; i++) {
		printerr_va("Non-option argument %s discarded.\n", argv[i]);
	}
	return 0;
}

const struct CLI_OPTS *cli_get_opts(void) {
	return &cli_opts;
}
