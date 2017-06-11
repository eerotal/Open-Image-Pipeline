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

#include "oip/abi/output.h"
#include "oip/plugin.h"
#include "oip/plugin.h"
#include "oip/cache.h"
#include "oip/file.h"
#include "oipbuildinfo/oipbuildinfo.h"

#include "cli_priv.h"

static PLUGIN **plugins;
static unsigned int plugin_count = 0;
static unsigned long long int plugin_last_uid = 0;

static unsigned int plugin_gen_uid_int(void);
static char *plugin_get_uid_str(PLUGIN *plugin);
static int plugin_data_append(PLUGIN *plugin);

static unsigned int plugin_gen_uid_int(void) {
	return plugin_last_uid++;
}

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
		printerrno("plugin: realloc(): ");
		plugin_count--;
		return 1;
	}
	plugins = tmp_plugins;

	// Allocate memory for plugin data.
	errno = 0;
	plugins[plugin_count - 1] = calloc(1, sizeof(PLUGIN));
	if (!plugins[plugin_count - 1]) {
		printerrno("plugin: calloc(): ");
		plugin_count--;
		errno = 0;
		tmp_plugins = realloc(plugins, sizeof(PLUGIN*)*plugin_count);
		if (!tmp_plugins) {
			printerrno("plugin: realloc(): ");
		} else {
			plugins = tmp_plugins;
		}
		return 1;
	}

	// Copy plugin data pointers.
	memcpy(plugins[plugin_count - 1], plugin, sizeof(PLUGIN));
	return 0;
}

int plugin_load(const char *dirpath, const char *name) {
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
	int ret = 0;

	unsigned int libfname_len = 0;
	unsigned int params_struct_name_len = 0;

	printverb_va("Loading plugin %s from directory %s.\n", name, dirpath);

	// Set plugin data struct to all zeroes initially.
	memset(&plugin, 0, sizeof(PLUGIN));

	// Construct the plugin .so filename.
	libfname_len = strlen("lib") + strlen(name) + strlen(".so") + 1;
	errno = 0;
	libfname = calloc(libfname_len, sizeof(char));
	if (libfname == NULL) {
		printerrno("plugin: calloc()");
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
			printerrno("plugin: calloc(): ");
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
			printerr_va("dlsym(): %s\n", dlret);
			free(path);
			free(params_struct_name);
			dlclose(plugin.p_handle);
			return 1;
		}

		ret = build_compare_critical(plugin.p_params->built_against, &OIP_BUILD_INFO);
		if (ret != BUILD_MATCH) {
			if (ret == BUILD_MISMATCH_ABI) {
				printerr_va("ABI version mismatch! %i vs. %i\n",
					plugin.p_params->built_against->abi, OIP_BUILD_INFO.abi);
			} else if (ret == BUILD_MISMATCH_DEBUG) {
				printerr("Debug build mismatch!");
				if (plugin.p_params->built_against->debug) {
					printf(" (Plugin: Debug) vs.");
				} else {
					printf(" (Plugin: Non-debug) vs.");
				}
				if (OIP_BUILD_INFO.debug) {
					printf(" (OIP: Debug)\n");
				} else {
					printf(" (OIP: Non-debug)\n");
				}
			}
			free(path);
			free(params_struct_name);
			dlclose(plugin.p_handle);
			return 1;
		}

		// Generate the plugin UID.
		plugin.uid = plugin_gen_uid_int();

		// Create plugin cache.
		cache_name = plugin_get_uid_str(&plugin);
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
		free(cache_name);

		// Append the plugin data to the plugin array.
		plugin_data_append(&plugin);

		// Set the flag_print_verbose value of the plugin.
		if (plugin.p_params->flag_print_verbose != NULL) {
			*plugin.p_params->flag_print_verbose = cli_get_opts()->opt_verbose;
		}

		// Run the setup function.
		plugin.p_params->plugin_setup();


		printverb_va("%s: Loaded!\n", path);
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
		printf("    Descr:           %s\n", plugins[i]->p_params->descr);
		printf("    Author:          %s\n", plugins[i]->p_params->author);
		printf("    Year:            %s\n", plugins[i]->p_params->year);
		build_print_version_info("    Built against:   ",
				plugins[i]->p_params->built_against);
		printf("    Args: \n");
		for (unsigned int arg = 0; arg < plugins[i]->argc; arg++) {
			printf("        %s: %s\n", plugins[i]->args[arg*2],
				plugins[i]->args[arg*2 + 1]);
		}
		printf("    Cache name:      %s\n", plugins[i]->p_cache->name);
		printf("    Cache path:      %s\n", plugins[i]->p_cache->path);
		printf("    UID:             %llu\n", plugins[i]->uid);
		printf("    Arg rev:         %llu\n", plugins[i]->arg_rev);
	}
	printf("\n");
}

int plugin_feed(const size_t index, struct PLUGIN_INDATA *in) {
	/*
	*  Feed data to a plugin. Returns one of the
	*  PLUGIN_STATUS_* values defined in oip/plugin.h.
	*/

	if (index < plugin_count) {
		return plugins[index]->p_params->plugin_process(in);
	}
	return PLUGIN_STATUS_ERROR;
}

int plugin_set_arg(const size_t index, const char *arg,
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
				p_args[i*2 + 1] = realloc(p_args[i*2 + 1],
						(strlen(value) + 1)*sizeof(*value));
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

int plugin_has_arg(const size_t index, const char *arg) {
	/*
	*  Check if the plugin loaded at index 'index'
	*  accepts 'arg' as an argument. Return 1 if it does
	*  and 0 otherwise.
	*/
	if (index < plugin_count) {
		for (size_t i = 0; i < plugins[index]->p_params->valid_args_count; i++) {
			if (strcmp(plugins[index]->p_params->valid_args[i], arg) == 0) {
				return 1;
			}
		}
	}
	return 0;
}

PLUGIN *plugin_get(const size_t index) {
	/*
	*  Return plugin data at index or NULL if
	*  the plugin doesn't exist.
	*/
	if (index < plugin_count) {
		return plugins[index];
	}
	return NULL;
}

unsigned int plugins_get_count(void) {
	return plugin_count;
}

char *plugin_get_uid_str(PLUGIN *plugin) {
	/*
	*  Return the plugin string identifier of the form 'name'-'uid'.
	*  Returns a pointer to a string on success and a NULL pointer
	*  on failure.
	*/

	char *ret = NULL;
	unsigned int uid_len = 0;

	if (plugin->uid == 0) {
		uid_len = 1;
	} else {
		uid_len = floor(log10(plugin->uid)) + 1;
	}

	errno = 0;
	ret = calloc(strlen(plugin->p_params->name) + 1 + uid_len + 1, sizeof(*ret));
	if (ret == NULL) {
		printerrno("plugin: calloc()");
		return NULL;
	}
	sprintf(ret, "%s-%llu", plugin->p_params->name, plugin->uid);
	return ret;
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
	for (size_t i = 0; i < plugin_count; i++) {
		if (plugins[i]) {
			plugins[i]->p_params->plugin_cleanup();
			dlclose(plugins[i]->p_handle);
			for (size_t a = 0; a < plugins[i]->argc*2; a++) {
				free(plugins[i]->args[a]);
			}
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
