#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dlfcn.h>
#include <string.h>
#include <math.h>
#include <errno.h>

#include "headers/plugin.h"
#include "cli_priv.h"
#include "plugin_priv.h"
#include "cache_priv.h"

static PLUGIN **plugins;
static unsigned int plugin_count = 0;
static unsigned long long int plugin_last_uid = 0;

static int plugin_data_append(PLUGIN *plugin);
static unsigned int plugin_gen_uid(void);

static int plugin_data_append(PLUGIN *plugin) {
	/*
	*  Copy the plugin data in 'plugin' to
	*  the plugin data array.
	*/

	// Extend data array.
	plugin_count++;
	errno = 0;
	PLUGIN **tmp_plugins = realloc(plugins, sizeof(PLUGIN**)*plugin_count);
	if (!tmp_plugins) {
		perror("plugin: realloc(): ");
		plugin_count--;
		return 1;
	}
	plugins = tmp_plugins;

	// Allocate memory for plugin data.
	errno = 0;
	plugins[plugin_count - 1] = calloc(1, sizeof(PLUGIN));
	if (!plugins[plugin_count - 1]) {
		perror("plugin: calloc(): ");
		plugin_count--;
		errno = 0;
		tmp_plugins = realloc(plugins, sizeof(PLUGIN*)*plugin_count);
		if (!tmp_plugins) {
			perror("plugin: realloc(): ");
		} else {
			plugins = tmp_plugins;
		}
		return 1;
	}

	// Copy plugin data pointers.
	memcpy(plugins[plugin_count - 1], plugin, sizeof(PLUGIN));
	return 0;
}

static unsigned int plugin_gen_uid(void) {
	return plugin_last_uid++;
}

int plugin_load(char *dirpath, char *name) {
	/*
	*  Load plugin with 'name' from the directory 'dirpath'.
	*/

	PLUGIN plugin;
	char *path = NULL;
	char *params_struct_name = NULL;
	char *cache_path = NULL;
	char *cache_name = NULL;
	char *dlret = NULL;

	unsigned int path_len = 0;
	unsigned int params_struct_name_len = 0;

	printf("plugin: Loading plugin %s from directory %s.\n", name, dirpath);

	// Set plugin data struct to all zeroes initially.
	memset(&plugin, 0, sizeof(PLUGIN));

	// Construct plugin path.
	path_len = strlen(dirpath) + strlen("lib") + strlen(name) + strlen(".so") + 1;
	errno = 0;
	path = calloc(path_len, sizeof(char));
	if (!path) {
		perror("plugin: calloc(): ");
		return 1;
	}
	snprintf(path, path_len, "%slib%s.so", dirpath, name);

	if (access(path, F_OK) == 0) {
		// Load shared library file.
		plugin.p_handle = dlopen(path, RTLD_NOW);
		if (!plugin.p_handle) {
			fprintf(stderr, "plugin: dlopen(): %s\n", dlerror());
			free(path);
			return 1;
		}

		// Construct plugin parameter structure name.
		params_struct_name_len = strlen(name) + strlen(PLUGIN_INFO_NAME_SUFFIX) + 1;
		errno = 0;
		params_struct_name = calloc(params_struct_name_len, sizeof(char));
		if (!params_struct_name) {
			perror("plugin: calloc(): ");
			free(path);
			dlclose(plugin.p_handle);
			return 1;
		}
		strcat(params_struct_name, name);
		strcat(params_struct_name, PLUGIN_INFO_NAME_SUFFIX);

		// Store the plugin parameter pointer in plugin.p_params.
		dlerror();
		plugin.p_params = dlsym(plugin.p_handle, params_struct_name);
		dlret = dlerror();
		if (dlret) {
			fprintf(stderr, "plugin: dlsym(): %s", dlret);
			free(path);
			free(params_struct_name);
			dlclose(plugin.p_handle);
			return 1;
		}

		// Create plugin cache.
		cache_name = plugin_get_full_identifier(name, plugin_count);
		if (cache_name == NULL) {
			printf("plugin: Failed to get plugin identifier.\n");
			free(path);
			free(params_struct_name);
			dlclose(plugin.p_handle);
			return 1;
		}
		plugin.cache_name = cache_name;

		cache_path = cache_create(cache_name);
		if (cache_path == NULL) {
			printf("plugin: Failed to create cache directory.\n");
			free(path);
			free(params_struct_name);
			dlclose(plugin.p_handle);
			return 1;
		} else {
			plugin.cache_path = cache_path;
		}

		// Generate the plugin UID.
		plugin.uid = plugin_gen_uid();

		// Append the plugin data to the plugin array.
		plugin_data_append(&plugin);

		// Run the setup function.
		plugin.p_params->plugin_setup();

		printf("plugin: %s: Loaded!\n", path);
		free(path);
		free(params_struct_name);
		return 0;
	}
	printf("plugin: Plugin doesn't exist.\n");
	return 1;
}

void print_plugin_config(void) {
	/*
	*  Print info about all loaded plugin to stdout.
	*/
	printf("\n");
	for (unsigned int i = 0; i < plugin_count; i++) {
		printf("%s:\n", plugins[i]->p_params->name);
		printf("    Descr:  %s\n", plugins[i]->p_params->descr);
		printf("    Author: %s\n", plugins[i]->p_params->author);
		printf("    Year:   %s\n", plugins[i]->p_params->year);
		printf("    Args: \n");
		for (unsigned int arg = 0; arg < plugins[i]->argc; arg++) {
			printf("        %s: %s\n", plugins[i]->args[arg*2],
				plugins[i]->args[arg*2 + 1]);
		}
		printf("    Cache name: %s\n", plugins[i]->cache_name);
		printf("    Cache path: %s\n", plugins[i]->cache_path);
		printf("    UID:        %llu\n", plugins[i]->uid);
		printf("    Arg rev:    %llu\n", plugins[i]->arg_rev);
	}
	printf("\n");
}

int plugin_feed(unsigned int index, const char **plugin_args,
		const unsigned int plugin_args_count,
		const IMAGE *img, IMAGE *res) {
	/*
	*  Feed image data to a plugin.
	*/
	unsigned int ret = 0;
	if (index < plugin_count) {
		ret = plugins[index]->p_params->plugin_process(img, res,
				plugin_args, plugin_args_count);
		if (ret == 0) {
			return 0;
		}
	}
	return 1;
}

unsigned int plugins_get_count(void) {
	return plugin_count;
}

int plugin_set_arg(const unsigned int index, const char *arg,
			const char *value) {
	/*
	*  Set the plugin argument 'arg' to 'value' for plugin
	*  with the index 'index'.
	*/
	unsigned int *p_argc = 0;
	char **p_args = NULL;
	char **tmp_args;

	if (index < plugin_count) {
		if (!plugin_has_arg(index, arg)) {
			return 1;
		}

		p_argc = &plugins[index]->argc;

		// Extend argument array.
		(*p_argc)++;
		tmp_args = realloc(plugins[index]->args, (*p_argc)*2*sizeof(char**));
		if (!tmp_args) {
			return 1;
		}
		/*
		*  Copy the argument array pointer into a temp variable
		*  and into the original struct too.
		*/
		p_args = tmp_args;
		plugins[index]->args = tmp_args;

		// Allocate space for strings.
		p_args[(*p_argc)*2 - 2] = malloc(strlen(arg)*sizeof(char) + 1);
		if (!p_args[(*p_argc)*2 - 2]) {
			(*p_argc)--;
			tmp_args = realloc(p_args, (*p_argc)*2);
			return 1;
		}
		p_args[(*p_argc)*2 - 1] = malloc(strlen(value)*sizeof(char) + 1);
		if (!p_args[(*p_argc)*2 - 1]) {
			(*p_argc)--;
			tmp_args = realloc(p_args, (*p_argc)*2);
			return 1;
		}

		// Copy data.
		strcpy(p_args[(*p_argc)*2 - 2], arg);
		strcpy(p_args[(*p_argc)*2 - 1], value);

		// Increment the argument revision.
		plugins[index]->arg_rev++;

		return 0;
	} else {
		return 1;
	}
}

int plugin_has_arg(const unsigned int index, const char *arg) {
	/*
	*  Check if the plugin loaded at index 'index'
	*  accepts 'arg' as an argument. Return 1 if it does
	*  and 0 otherwise.
	*/
	if (index < plugin_count) {
		for (unsigned int i = 0; i < plugins[index]->p_params->valid_args_count; i++) {
			if (strcmp(plugins[index]->p_params->valid_args[i], arg) == 0) {
				return 1;
			}
		}
	}
	return 0;
}

PLUGIN *plugin_get(unsigned int index) {
	/*
	*  Return plugin data at index or NULL if
	*  the plugin doesn't exist.
	*/
	if (index < plugin_count) {
		return plugins[index];
	}
	return NULL;
}

char *plugin_get_full_identifier(const char *name, unsigned int index) {
	/*
	*  Build a plugin indentifier string of the
	*  form <Plugin Name>-<Plugin Index>. This function
	*  returns a pointer to a new string on success and NULL
	*  on failure.
	*/

	unsigned int index_str_len = 0;
	char *index_str = NULL;
	char *identifier = NULL;

	if (index == 0) {
		index_str_len = 2;
	} else {
		index_str_len = floor(log10(index)) + 2;
	}

	// Allocate memory for the ID string.
	errno = 0;
	index_str = calloc(index_str_len, sizeof(char));
	if (index_str == NULL) {
		perror("plugin: calloc(): ");
		return NULL;
	}
	sprintf(index_str, "%i", index);

	// Allocate space for the full identifier.
	errno = 0;
	identifier = calloc(strlen(name) + 1 + strlen(index_str) + 1, sizeof(char));
	if (identifier == NULL) {
		perror("plugin: calloc(): ");
		free(index_str);
		return NULL;
	}
	strcat(identifier, name);
	strcat(identifier, "-");
	strcat(identifier, index_str);
	free(index_str);

	return identifier;
}

void plugins_cleanup(void) {
	/*
	*  Free memory allocated for plugin data and close
	*  opened library handles.
	*/
	printf("plugin: Freeing plugins...\n");
	for (unsigned int i = 0; i < plugin_count; i++) {
		if (plugins[i]) {
			plugins[i]->p_params->plugin_cleanup();
			free(plugins[i]->args);
			free(plugins[i]->cache_path);
			dlclose(plugins[i]->p_handle);
			free(plugins[i]);
			plugins[i] = NULL;
		}
	}
	if (plugins) {
		printf("plugin: Freeing plugin data array...\n");
		free(plugins);
	}
	printf("plugin: All plugins free'd!\n");

	if (!cli_get_opts()->opt_preserve_cache) {
		if (cache_delete_all() != 0) {
			printf("plugin: Failed to delete cache files.\n");
		}
	} else {
		printf("plugin: Skipping cache deletion because cache preservation is enabled.\n");
	}

	printf("plugin: Cleanup done!\n");
}
