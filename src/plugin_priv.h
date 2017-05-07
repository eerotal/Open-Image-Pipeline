#ifndef PLUGIN_PRIV_INCLUDED
	#define PLUGIN_PRIV_INCLUDED

	#include "headers/plugin.h"

	int plugin_load(char *dirpath, char *name);
	void print_plugin_config(void);
	int plugin_feed(unsigned int index, const IMAGE *img, IMAGE *res);
	unsigned int plugins_get_count(void);
	void plugins_cleanup(void);
#endif
