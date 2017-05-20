#ifndef CLI_PRIV_INCLUDED
	#define CLI_PRIV_INCLUDED

	#define CLI_OPT_ENABLED 1
	#define CLI_OPT_DISABLED 0

	int cli_parse_opts(int argc, char **argv);
	void cli_opts_cleanup(void);

	struct CLI_OPTS {
		unsigned int opt_verbose;
		unsigned int opt_dryrun;
		char *opt_image_path;
	} cli_opts;
#endif
