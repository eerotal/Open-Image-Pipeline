#ifndef PLUGIN_PRIV_INCLUDED
	#define PLUGIN_PRIV_INCLUDED

	#include "headers/plugin.h"

	int plugin_load(char *dirpath, char *name);
	void print_plugin_config(void);
	int plugin_feed(unsigned int index, const char **plugin_args,
			const unsigned int plugin_args_count,
			const IMAGE *img, IMAGE *res);
	int plugin_set_arg(const unsigned int index, const char *arg, const char *value);
	int plugin_has_arg(const unsigned int index, const char *arg);
	PLUGIN *plugin_get(unsigned int index);
	unsigned int plugins_get_count(void);
	char *plugin_get_full_identifier(const char *name, unsigned int index);
	void plugins_cleanup(void);
#endif
