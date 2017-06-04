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
		char *opt_config_file;
	};

	int cli_parse_opts(int argc, char **argv);
	const struct CLI_OPTS *cli_get_opts(void);
#endif
