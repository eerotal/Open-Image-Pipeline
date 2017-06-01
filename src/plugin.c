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

#define PRINT_IDENTIFIER "plugin"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dlfcn.h>
#include <string.h>
#include <math.h>
#include <errno.h>

#include "headers/output.h"
#include "headers/plugin.h"
#include "cli_priv.h"
#include "plugin_priv.h"
#include "cache_priv.h"
#include "file.h"

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
	CACHE *p_cache;
	char *cache_name = NULL;
	char *libfname = NULL;
	char *path = NULL;
	char *params_struct_name = NULL;
	char *dlret = NULL;

	unsigned int libfname_len = 0;
	unsigned int params_struct_name_len = 0;

	printinfo_va("Loading plugin %s from directory %s.\n", name, dirpath);

	// Set plugin data struct to all zeroes initially.
	memset(&plugin, 0, sizeof(PLUGIN));

	// Construct the plugin .so filename.
	libfname_len = strlen("lib") + strlen(name) + strlen(".so") + 1;
	errno = 0;
	libfname = calloc(libfname_len, sizeof(char));
	if (libfname == NULL) {
		perror("plugin: calloc()");
		return 1;
	}
	sprintf(libfname, "lib%s.so", name);

	// Construct plugin filepath.
	path = file_path_join(dirpath, libfname);
	free(libfname);
	if (path == NULL) {
		printerr("Failed to create plugin shared library path.\n");
		return 1;
	}

	if (access(path, F_OK) == 0) {
		// Load shared library file.
		plugin.p_handle = dlopen(path, RTLD_NOW);
		if (!plugin.p_handle) {
			printerr_va("dlopen(): %s\n", dlerror());
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
			printerr_va("dlsym(): %s", dlret);
			free(path);
			free(params_struct_name);
			dlclose(plugin.p_handle);
			return 1;
		}

		// Create plugin cache.
		cache_name = plugin_get_full_identifier(name, plugin_count);
		if (cache_name == NULL) {
			printerr("Failed to get plugin identifier.\n");
			free(path);
			free(params_struct_name);
			dlclose(plugin.p_handle);
			return 1;
		}

		p_cache = cache_create(cache_name);
		if (p_cache == NULL) {
			printerr("Failed to create plugin cache.\n");
			free(path);
			free(params_struct_name);
			dlclose(plugin.p_handle);
			return 1;
		}
		plugin.p_cache = p_cache;

		// Generate the plugin UID.
		plugin.uid = plugin_gen_uid();

		// Append the plugin data to the plugin array.
		plugin_data_append(&plugin);

		// Set the flag_print_verbose value of the plugin.
		if (plugin.p_params->flag_print_verbose != NULL) {
			*plugin.p_params->flag_print_verbose = cli_get_opts()->opt_verbose;
		}

		// Run the setup function.
		plugin.p_params->plugin_setup();


		printinfo_va("%s: Loaded!\n", path);
		free(path);
		free(params_struct_name);
		return 0;
	}
	printerr("Plugin doesn't exist.\n");
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
		printf("    Cache name: %s\n", plugins[i]->p_cache->name);
		printf("    Cache path: %s\n", plugins[i]->p_cache->path);
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
	*  Set the plugin argument 'arg' to 'value' for plugin at 'index'.
	*  Returns 0 on success and 1 on failure.
	*/
	unsigned int p_argc = 0;
	char **p_args = NULL;

	if (index < plugin_count) {
		if (!plugin_has_arg(index, arg)) {
			return 1;
		}

		p_argc = plugins[index]->argc;
		p_args = plugins[index]->args;

		// Check if the argument already exists and modify it if it does.
		for (unsigned int i = 0; i < p_argc; i++) {
			if (strcmp(p_args[i*2], arg) == 0) {
				printverb_va("Plugin arg '%s' exists. Modifying it.\n", arg);
				p_args[i*2 + 1] = realloc(p_args[i*2 + 1], (strlen(value) + 1)*sizeof(*value));
				if (p_args[i*2 + 1] == NULL) {
					return 1;
				}
				strcpy(p_args[i*2 + 1], value);
				plugins[index]->args = p_args;
				return 0;
			}
		}

		printverb_va("Adding plugin arg '%s'.\n", arg);

		// Extend the argument array.
		p_argc++;
		p_args = realloc(p_args, p_argc*2*sizeof(*p_args));
		if (p_args == NULL) {
			return 1;
		}

		// Allocate memory for strings.
		p_args[p_argc*2 - 2] = malloc((strlen(arg) + 1)*sizeof(*arg));
		if (p_args[p_argc*2 - 2] == NULL) {
			p_argc--;
			p_args = realloc(p_args, p_argc*2*sizeof(*p_args));
			plugins[index]->args = p_args;
			return 1;
		}

		p_args[p_argc*2 - 1] = malloc((strlen(value) + 1)*sizeof(*value));
		if (p_args[p_argc*2 - 2] == NULL) {
			p_argc--;
			p_args = realloc(p_args, p_argc*2*sizeof(*p_args));
			plugins[index]->args = p_args;
			return 1;
		}

		strcpy(p_args[p_argc*2 - 2], arg);
		strcpy(p_args[p_argc*2 - 1], value);

		// Update values in the plugin struct.
		plugins[index]->args = p_args;
		plugins[index]->argc = p_argc;
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

int plugins_setup(void) {
	/*
	*  Setup the plugin system.
	*/

	// Setup the cache system.
	if (cache_setup() != 0) {
		printerr("Failed to setup the cache system.\n");
		return 1;
	}
	return 0;
}

void plugins_cleanup(void) {
	/*
	*  Free memory allocated for plugin data and close
	*  opened library handles.
	*/
	printverb("Freeing plugins...\n");
	for (unsigned int i = 0; i < plugin_count; i++) {
		if (plugins[i]) {
			plugins[i]->p_params->plugin_cleanup();
			dlclose(plugins[i]->p_handle);
			free(plugins[i]->args);
			free(plugins[i]);
			plugins[i] = NULL;
		}
	}
	if (plugins) {
		printverb("Freeing plugin data array...\n");
		free(plugins);
	}
	printverb("All plugins free'd!\n");
	cache_cleanup(!cli_get_opts()->opt_preserve_cache);
}
