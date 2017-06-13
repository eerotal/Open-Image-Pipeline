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

#include "oipcore/abi/output.h"
#include "oipcore/plugin.h"
#include "oipcore/plugin.h"
#include "oipcore/cache.h"
#include "oipcore/file.h"
#include "oipbuildinfo/oipbuildinfo.h"

#include "cli_priv.h"

PTRARRAY_TYPE_DEF(PLUGIN);

PTRARRAY_TYPE(PLUGIN) *plugins = NULL;
static unsigned long long int plugin_last_uid = 0;

static unsigned int plugin_gen_uid_int(void);
static char *plugin_get_uid_str(PLUGIN *plugin);
static int plugin_data_append(PLUGIN *plugin);
static void plugin_free(PLUGIN *plugin);
static void plugin_free_wrapper(void *plugin);

static unsigned int plugin_gen_uid_int(void) {
	return plugin_last_uid++;
}

static int plugin_data_append(PLUGIN *plugin) {
	/*
	*  Add 'plugin' to the plugins PTRARRAY. Returns 0 on
	*  success or 1 on failure.
	*/
	if (!ptrarray_put_data((PTRARRAY_TYPE(void)*) plugins, plugin, sizeof(*plugin))) {
		printerr("Failed to copy plugin data to the plugins PTRARRAY.\n");
		return 1;
	}
	return 0;
}

int plugin_load(const char *dirpath, const char *name) {
	/*
	*  Load plugin with 'name' from the directory 'dirpath'.
	*/

	PLUGIN plugin;
	char *cache_name = NULL;
	char *libfname = NULL;
	char *path = NULL;
	char *info_struct_name = NULL;
	char *dlret = NULL;
	int ret = 0;

	size_t libfname_len = 0;
	size_t info_struct_name_len = 0;

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
		free(path);
		if (!plugin.p_handle) {
			printerr_va("dlopen(): %s\n", dlerror());
			return 1;
		}

		// Construct the plugin info structure name.
		info_struct_name_len = strlen(name) + strlen(PLUGIN_INFO_NAME_SUFFIX) + 1;
		errno = 0;
		info_struct_name = calloc(info_struct_name_len, sizeof(char));
		if (!info_struct_name) {
			printerrno("plugin: calloc(): ");
			dlclose(plugin.p_handle);
			return 1;
		}
		sprintf(info_struct_name, "%s"PLUGIN_INFO_NAME_SUFFIX, name);

		// Store the plugin parameter pointer in plugin.p_params.
		dlerror();
		plugin.p_params = dlsym(plugin.p_handle, info_struct_name);
		free(info_struct_name);
		dlret = dlerror();
		if (dlret) {
			printerr_va("dlsym(): %s\n", dlret);
			dlclose(plugin.p_handle);
			return 1;
		}

		// Check for build mismatches.
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
			dlclose(plugin.p_handle);
			return 1;
		}

		// Generate the plugin UID.
		plugin.uid = plugin_gen_uid_int();

		// Create plugin cache.
		cache_name = plugin_get_uid_str(&plugin);
		if (!cache_name) {
			printerr("Failed to get plugin identifier.\n");
			dlclose(plugin.p_handle);
			return 1;
		}

		plugin.p_cache = cache_create(cache_name);
		free(cache_name);
		if (!plugin.p_cache) {
			printerr("Failed to create plugin cache.\n");
			dlclose(plugin.p_handle);
			return 1;
		}

		plugin.args = (PTRARRAY_TYPE(char)*) ptrarray_create(&free);
		if (!plugin.args) {
			dlclose(plugin.p_handle);
			cache_destroy(plugin.p_cache, 0);
			return 1;
		}

		// Append the plugin data to the plugin array.
		plugin_data_append(&plugin);

		// Set the flag_print_verbose value of the plugin.
		if (plugin.p_params->flag_print_verbose) {
			*plugin.p_params->flag_print_verbose = cli_get_opts()->opt_verbose;
		}

		// Run the setup function.
		plugin.p_params->plugin_setup();
		return 0;
	}
	free(path);
	printerr("Plugin doesn't exist.\n");
	return 1;
}

void print_plugin_config(void) {
	/*
	*  Print info about all loaded plugin to stdout.
	*/
	printf("\n");
	for (unsigned int i = 0; i < plugins->ptrc; i++) {
		printf("%s:\n", plugins->ptrs[i]->p_params->name);
		printf("    Descr:           %s\n", plugins->ptrs[i]->p_params->descr);
		printf("    Author:          %s\n", plugins->ptrs[i]->p_params->author);
		printf("    Year:            %s\n", plugins->ptrs[i]->p_params->year);
		build_print_version_info("    Built against:   ",
				plugins->ptrs[i]->p_params->built_against);
		printf("    Args: \n");
		for (size_t arg = 0; arg < plugins->ptrs[i]->args->ptrc; arg += 2) {
			printf("        %s: %s\n", plugins->ptrs[i]->args->ptrs[arg],
				plugins->ptrs[i]->args->ptrs[arg + 1]);
		}
		printf("    Cache name:      %s\n", plugins->ptrs[i]->p_cache->name);
		printf("    Cache path:      %s\n", plugins->ptrs[i]->p_cache->path);
		printf("    UID:             %llu\n", plugins->ptrs[i]->uid);
		printf("    Arg rev:         %llu\n", plugins->ptrs[i]->arg_rev);
	}
	printf("\n");
}

int plugin_feed(const size_t index, struct PLUGIN_INDATA *in) {
	/*
	*  Feed data to a plugin. Returns one of the
	*  PLUGIN_STATUS_* values defined in oip/plugin.h.
	*/

	if (index < plugins->ptrc) {
		return plugins->ptrs[index]->p_params->plugin_process(in);
	}
	return PLUGIN_STATUS_ERROR;
}

int plugin_set_arg(const size_t index, char *arg, char *value) {
	/*
	*  Set the plugin argument 'arg' to 'value' for plugin at 'index'.
	*  Returns 0 on success and 1 on failure.
	*/

	int ret = -1;
	char *tmp_str = NULL;
	if (index < plugins->ptrc) {
		if (!plugin_has_arg(index, arg)) {
			return 1;
		}

		// If the argument exists, modify it.
		ret = ptrarray_get_ptr_index((PTRARRAY_TYPE(void)*) plugins->ptrs[index]->args, arg);
		if (ret >= 0) {
			printverb_va("Plugin arg '%s' exists. Modifying it.\n", arg);
			tmp_str = realloc(plugins->ptrs[index]->args->ptrs[ret + 1],
				(strlen(value) + 1)*sizeof(*value));
			if (!tmp_str) {
				return 1;
			}
			strcpy(tmp_str, value);
			plugins->ptrs[index]->args->ptrs[ret + 1] = tmp_str;
			return 0;
		}

		// Add a new argument.
		printverb_va("Adding plugin arg '%s'.\n", arg);
		tmp_str = ptrarray_put_data((PTRARRAY_TYPE(void)*) plugins->ptrs[index]->args,
				arg, (strlen(arg) + 1)*sizeof(*arg));
		if (!tmp_str) {
			return 1;
		}
		tmp_str = ptrarray_put_data((PTRARRAY_TYPE(void)*) plugins->ptrs[index]->args,
				value, (strlen(value) + 1)*sizeof(*value));
		if (!tmp_str) {
			ptrarray_pop_ptr((PTRARRAY_TYPE(void)*) plugins->ptrs[index]->args, arg, 0);
			return 1;
		}
		plugins->ptrs[index]->arg_rev++;
		return 0;
	}
	return 1;
}

int plugin_has_arg(const size_t index, const char *arg) {
	/*
	*  Check if the plugin loaded at index 'index'
	*  accepts 'arg' as an argument. Return 1 if it does
	*  and 0 otherwise.
	*/
	if (index < plugins->ptrc) {
		for (size_t i = 0; i < plugins->ptrs[index]->p_params->valid_args_count; i++) {
			if (strcmp(plugins->ptrs[index]->p_params->valid_args[i], arg) == 0) {
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
	if (index < plugins->ptrc) {
		return plugins->ptrs[index];
	}
	return NULL;
}

size_t plugins_get_count(void) {
	return plugins->ptrc;
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

static void plugin_free(PLUGIN *plugin) {
	/*
	*  Free the resources allocated to a plugin.
	*/
	plugin->p_params->plugin_cleanup();
	dlclose(plugin->p_handle);
	ptrarray_free_ptrs((PTRARRAY_TYPE(void)*) plugin->args);
	ptrarray_free((PTRARRAY_TYPE(void)*) plugin->args);
	free(plugin);
}

static void plugin_free_wrapper(void *plugin) {
	/*
	*  A plugin_free() wrapper for use as a
	*  PTRARRAY freeing function.
	*/
	plugin_free((PLUGIN*) plugin);
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

	// Setup the plugins PTRARRAY.
	plugins = (PTRARRAY_TYPE(PLUGIN)*) ptrarray_create(&plugin_free_wrapper);
	if (!plugins) {
		printerr("Failed to setup plugins PTRARRAY.\n");
		cache_cleanup(0);
		return 1;
	}
	return 0;
}

void plugins_cleanup(void) {
	/*
	*  Free memory allocated for plugin data and close
	*  opened library handles.
	*/
	if (plugins) {
		printverb("Freeing allocated plugin resources...\n");
		ptrarray_free_ptrs((PTRARRAY_TYPE(void)*) plugins);
		ptrarray_free((PTRARRAY_TYPE(void)*) plugins);
		printverb("All plugins free'd!\n");
	}
	cache_cleanup(!cli_get_opts()->opt_preserve_cache);
}
