#ifndef PLUGIN_PRIV_INCLUDED
	#define PLUGIN_PRIV_INCLUDED

	#include "headers/plugin.h"

	int plugin_load(char *dirpath, char *name);
	void print_plugin_config(void);
	int plugin_feed(unsigned int index, const char **plugin_args,
			const unsigned int plugin_args_count,
			const IMAGE *img, IMAGE *res);
	char **plugin_args_get(unsigned int index);
	unsigned int plugin_args_get_count(unsigned int index);
	int plugin_set_arg(const unsigned int index, const char *arg, const char *value);
	int plugin_has_arg(const unsigned int index, const char *arg);
	unsigned int plugins_get_count(void);
	void plugins_cleanup(void);
#endif
