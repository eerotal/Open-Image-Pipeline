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

#ifndef PLUGIN_INCLUDED
	#define PLUGIN_INCLUDED

	#include "imgutil/imgutil.h"
	#include "buildinfo/build.h"

	#define PLUGIN_INFO_NAME_SUFFIX "_plugin_info"
	#define PLUGIN_INFO_NAME(NAME) ( NAME ## _plugin_info )

	#define PLUGIN_STATUS_ERROR   -1
	#define PLUGIN_STATUS_DONE     2

	struct PLUGIN_INDATA {
		IMAGE *src;
		IMAGE *dst;
		char **args;
		int argc;
		void (*set_progress)(const unsigned int progress);
	};

	typedef struct STRUCT_PLUGIN_INFO {
		const char *name;
		const char *descr;
		const char *author;
		const char *year;
		const struct BUILD_INFO_STRUCT *built_against;

		const char **valid_args;
		const unsigned int valid_args_count;

		unsigned int *flag_print_verbose;

		int (*plugin_process)(struct PLUGIN_INDATA *in);
		int (*plugin_setup)(void);
		void (*plugin_cleanup)(void);
	} PLUGIN_INFO;
#endif
