#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dlfcn.h>
#include <string.h>

#include "headers/plugin.h"
#include "plugin_priv.h"

static PLUGIN **plugins;
static unsigned int plugin_count = 0;

static int plugin_data_append(PLUGIN *plugin);

static int plugin_data_append(PLUGIN *plugin) {
	/*
	*  Copy the plugin data in 'plugin' to
	*  the plugin data array.
	*/

	// Extend data array.
	plugin_count++;
	PLUGIN **tmp_plugins = realloc(plugins, sizeof(PLUGIN**)*plugin_count);
	if (!tmp_plugins) {
		printf("realloc(): Failed to extend plugin array.\n");
		plugin_count--;
		return 1;
	}
	plugins = tmp_plugins;

	// Allocate memory for plugin data.
	plugins[plugin_count - 1] = calloc(1, sizeof(PLUGIN));
	if (!plugins[plugin_count - 1]) {
		printf("malloc(): Failed to allocate memory for plugin data.\n");
		plugin_count--;
		tmp_plugins = realloc(plugins, sizeof(PLUGIN*)*plugin_count);
		if (!tmp_plugins) {
			printf("realloc(): Failed to shrink plugin array.\n");
		} else {
			plugins = tmp_plugins;
		}
		return 1;
	}

	// Copy plugin data.
	plugins[plugin_count - 1]->p_handle = plugin->p_handle;
	plugins[plugin_count - 1]->p_params = plugin->p_params;
	return 0;
}

int plugin_load(char *dirpath, char *name) {
	/*
	*  Load plugin with 'name' from the directory 'dirpath'.
	*/

	PLUGIN plugin;
	char *path = NULL;
	char *params_struct_name = NULL;

	unsigned int path_len = 0;
	unsigned int params_struct_name_len = 0;

	printf("Loading plugin %s from directory %s.\n", name, dirpath);

	// Construct plugin path.
	path_len = strlen(dirpath) + strlen("lib") + strlen(name) + strlen(".so") + 1;
	path = calloc(path_len, sizeof(char));
	if (!path) {
		printf("malloc(): Failed to allocate memory for plugin path string.\n");
		return 1;
	}
	snprintf(path, path_len, "%slib%s.so", dirpath, name);

	if (access(path, F_OK) == 0) {
		// Load shared library file.
		plugin.p_handle = dlopen(path, RTLD_NOW);
		if (!plugin.p_handle) {
			printf("dlopen(): %s\n", dlerror());
			free(path);
			return 1;
		}

		// Construct plugin parameter structure name.
		params_struct_name_len = strlen(name) + strlen(PLUGIN_PARAMS_NAME_SUFFIX) + 1;
		params_struct_name = calloc(params_struct_name_len, sizeof(char));
		if (!params_struct_name) {
			printf("malloc(): Failed to allocate memory for params_struct_name.\n");
			free(path);
			dlclose(plugin.p_handle);
			return 1;
		}
		strcat(params_struct_name, name);
		strcat(params_struct_name, PLUGIN_PARAMS_NAME_SUFFIX);

		// Store the plugin parameter pointer in plugin.p_params.
		dlerror();
		plugin.p_params = dlsym(plugin.p_handle, params_struct_name);
		if (dlerror()) {
			printf("dlsym(): No plugin parameters found. Unable to load plugin.\n");
			free(path);
			free(params_struct_name);
			dlclose(plugin.p_handle);
			return 1;
		}

		// Append the plugin data to the plugin array.
		plugin_data_append(&plugin);

		// Run the setup function.
		plugin.p_params->plugin_setup();

		printf("%s: Loaded!\n", path);
		free(path);
		free(params_struct_name);
		return 0;
	}
	printf("Plugin doesn't exist.\n");
	return 1;
}

void print_plugin_config(void) {
	/*
	*  Print info about all loaded plugin to stdout.
	*/
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
	}
}

int plugin_feed(unsigned int index, const char **plugin_args,
		const unsigned int plugin_args_count,
		const IMAGE *img, IMAGE *res) {
	/*
	*  Feed image data to a plugin.
	*/
	if (index < plugin_count) {
		plugins[index]->p_params->plugin_process(img, res,
				plugin_args, plugin_args_count);
		return 0;
	} else {
		return 1;
	}
}

unsigned int plugins_get_count(void) {
	return plugin_count;
}

char **plugin_args_get(unsigned int index) {
	if (index < plugin_count) {
		return plugins[index]->args;
	}
	return NULL;
}

unsigned int plugin_args_get_count(unsigned int index) {
	if (index < plugin_count) {
		return plugins[index]->argc;
	}
	return 0;
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
		p_args = plugins[index]->args;

		// Extend argument array.
		(*p_argc)++;
		tmp_args = realloc(p_args, (*p_argc)*2);
		if (!tmp_args) {
			return 1;
		}
		p_args = tmp_args;
		/*
		* Copy the argument array pointer back to the
		* original structure too.
		*/
		plugins[index]->args = p_args;

		// Allocate space for strings.
		p_args[(*p_argc)*2 - 2] = malloc(strlen(arg)*sizeof(char));
		if (!p_args[(*p_argc)*2 - 2]) {
			(*p_argc)--;
			tmp_args = realloc(p_args, (*p_argc)*2);
			return 1;
		}
		p_args[(*p_argc)*2 - 1] = malloc(strlen(value)*sizeof(char));
		if (!p_args[(*p_argc)*2 - 1]) {
			(*p_argc)--;
			tmp_args = realloc(p_args, (*p_argc)*2);
			return 1;
		}

		// Copy data.
		strcpy(p_args[(*p_argc)*2 - 2], arg);
		strcpy(p_args[(*p_argc)*2 - 1], value);
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

void plugins_cleanup(void) {
	/*
	*  Free memory allocated for plugin data and close
	*  opened library handles.
	*/
	printf("Freeing plugins...\n");
	for (unsigned int i = 0; i < plugin_count; i++) {
		if (plugins[i]) {
			plugins[i]->p_params->plugin_cleanup();
			dlclose(plugins[i]->p_handle);
			free(plugins[i]);
		}
	}
	if (plugins) {
		free(plugins);
	}
	printf("All plugins free'd.\n");
}
