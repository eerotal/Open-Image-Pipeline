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

#ifndef PLUGIN_PRIV_INCLUDED
	#define PLUGIN_PRIV_INCLUDED

	#include "headers/plugin.h"
	#include "cache_priv.h"

	typedef struct STRUCT_PLUGIN {
		PLUGIN_INFO *p_params;
		CACHE *p_cache;
		void *p_handle;

		char **args;
		unsigned int argc;

		unsigned long long int arg_rev;
		unsigned long long int uid;
	} PLUGIN;

	int plugin_load(char *dirpath, char *name);
	void print_plugin_config(void);
	int plugin_feed(unsigned int index, struct PLUGIN_INDATA *in);
	int plugin_set_arg(const unsigned int index, const char *arg, const char *value);
	int plugin_has_arg(const unsigned int index, const char *arg);
	PLUGIN *plugin_get(unsigned int index);
	unsigned int plugins_get_count(void);
	int plugins_setup(void);
	void plugins_cleanup(void);
#endif
