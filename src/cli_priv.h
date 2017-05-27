#ifndef CLI_PRIV_INCLUDED
	#define CLI_PRIV_INCLUDED

	#define CLI_OPT_ENABLED 1
	#define CLI_OPT_DISABLED 0

	#include <pthread.h>

	int cli_parse_opts(int argc, char **argv);
	pthread_t *cli_shell_init(void);
	void cli_opts_cleanup(void);

	struct CLI_OPTS {
		unsigned int opt_preserve_cache;
		unsigned int opt_verbose;
		unsigned int opt_dryrun;
	};

	int cli_parse_opts(int argc, char **argv);
	const struct CLI_OPTS *cli_get_opts(void);
#endif
