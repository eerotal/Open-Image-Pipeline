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
