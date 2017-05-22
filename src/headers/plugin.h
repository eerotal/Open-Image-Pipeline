#ifndef PLUGIN_INCLUDED
	#define PLUGIN_INCLUDED

	#include "imgutil/imgutil.h"

	#define PLUGIN_PARAMS_NAME_SUFFIX "_plugin_params"
	#define PLUGIN_PARAMS_NAME(NAME) ( NAME ## _plugin_params )

	typedef struct STRUCT_PLUGIN_PARAMS {
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
	} PLUGIN_PARAMS;

	typedef struct STRUCT_PLUGIN {
		PLUGIN_PARAMS *p_params;
		void *p_handle;
		char **args;
		unsigned int argc;
		unsigned int dirty_args;
		char *cache_path;
	} PLUGIN;
#endif
