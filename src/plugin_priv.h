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
	const PLUGIN_INFO *plugin_get_params(unsigned int index);
	unsigned int plugins_get_count(void);
	char *plugin_get_full_identifier(const char *name, unsigned int index);
	char *plugin_get_cache_path(unsigned int index);
	char *plugin_get_cache_name(unsigned int index);
	void plugins_cleanup(void);
#endif
