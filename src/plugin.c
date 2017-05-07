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
	PLUGIN **tmp_plugins = realloc(plugins, sizeof(PLUGIN*)*plugin_count);
	if (!tmp_plugins) {
		printf("realloc(): Failed to extend plugin array.\n");
		plugin_count--;
		return 1;
	}
	plugins = tmp_plugins;

	// Allocate memory for plugin data.
	plugins[plugin_count - 1] = malloc(sizeof(PLUGIN));
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
	path = malloc(path_len*sizeof(char));
	if (!path) {
		printf("malloc(): Failed to allocate memory for plugin path string.\n");
		return 1;
	}
	memset(path, 0, path_len*sizeof(char));
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
		params_struct_name = malloc(params_struct_name_len*sizeof(char));
		memset(params_struct_name, 0, params_struct_name_len*sizeof(char));
		if (!params_struct_name) {
			printf("malloc(): Failed to allocate memory for params_struct_name.\n");
			free(path);
			dlclose(plugin.p_handle);
			return 1;
		}
		strcat(params_struct_name, name);
		strcat(params_struct_name, PLUGIN_PARAMS_NAME_SUFFIX);

		// Store plugin parameter pointer in plugin.p_params.
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
	}
	return 0;
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
		printf("    Args:   %s\n", plugins[i]->p_params->args);
	}
}

int plugin_feed(unsigned int index, const IMAGE *img, IMAGE *res) {
	if (index < plugin_count) {
		plugins[index]->p_params->plugin_process(img, res);
		return 0;
	} else {
		return 1;
	}
}

unsigned int plugins_get_count(void) {
	return plugin_count;
}

void plugins_cleanup(void) {
	/*
	*  Free memory allocated for plugin data and close
	*  opened library handles.
	*/
	for (unsigned int i = 0; i < plugin_count; i++) {
		if (plugins[i]) {
			plugins[i]->p_params->plugin_cleanup();
			dlclose(plugins[i]->p_handle);
			printf("Closed plugin handle.\n");
			free(plugins[i]);
		}
	}
	free(plugins);
	printf("All plugins free'd.\n");
}
