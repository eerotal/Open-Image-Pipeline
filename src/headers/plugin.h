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

	#define PLUGIN_INFO_NAME_SUFFIX "_plugin_params"
	#define PLUGIN_INFO_NAME(NAME) ( NAME ## _plugin_params )

	typedef struct STRUCT_PLUGIN_INFO {
		char *name;
		char *descr;
		char *author;
		char *year;

		char **valid_args;
		unsigned int valid_args_count;

		int (*plugin_process)(const IMAGE *img, IMAGE *img_dest,
					const char **plugin_args,
					const unsigned int plugin_args_count);
		int (*plugin_setup)(void);
		void (*plugin_cleanup)(void);
	} PLUGIN_INFO;

	typedef struct STRUCT_PLUGIN {
		PLUGIN_INFO *p_params;
		void *p_handle;
		char **args;
		unsigned int argc;
		char *cache_path;
		char *cache_name;
		unsigned long long int arg_rev;
		unsigned long long int uid;
	} PLUGIN;
#endif
