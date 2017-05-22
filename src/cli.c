#include <stdio.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>

#include "cli_priv.h"

#define CLI_GETOPT_OPTS "pvi:o:"

static struct CLI_OPTS cli_opts;

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
				cli_opts.opt_in_path = optarg;
				break;
			case 'o':
				cli_opts.opt_out_path = optarg;
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
