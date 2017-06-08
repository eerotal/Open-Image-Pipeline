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

#define PRINT_IDENTIFIER "cache"

#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>
#include <math.h>
#include <errno.h>
#include <unistd.h>
#include <time.h>

#include "headers/output.h"
#include "file.h"
#include "configloader_priv.h"
#include "cache_priv.h"
#include "ptrarray_priv.h"

#define CACHE_PERMISSIONS S_IRWXU

static char *cache_root = NULL;
static size_t cache_default_max_files = 0;

static PTRARRAY_TYPE(CACHE) *caches = NULL;

static void cache_db_file_free(CACHE_FILE *cache_file);
static void cache_db_file_free_wrapper(void *cache_file);
static CACHE_FILE *cache_db_file_create(const CACHE *cache,
					const char *fname);

static int cache_db_file_get_index(const CACHE *cache,
					const char *fname);
static int cache_db_file_get_index_oldest(const CACHE *cache);

void cache_dump(const CACHE *cache) {
	/*
	*  Dump info about 'cache' to STDOUT.
	*/
	printf("Cache '%s':\n", cache->name);
	printf("  Name:      %s\n", cache->name);
	printf("  Path:      %s\n", cache->path);
	printf("  Max files: %u\n", cache->max_files);
	printf("  Files:\n");
	for (size_t i = 0; i < cache->db->ptrc; i++) {
		printf("    %s : %s\n", cache->db->ptrs[i]->fname, cache->db->ptrs[i]->fpath);
	}
}

void cache_dump_all(void) {
	/*
	*  Dump info about all caches to STDOUT.
	*/
	for (size_t i = 0; i < caches->ptrc; i++) {
		cache_dump(caches->ptrs[i]);
	}
}


static void cache_db_file_free_wrapper(void *cache_file) {
	/*
	*  Wrapper for cache_db_file_free(). This is used
	*  as the freeing function for the PTRARRAY.
	*/
	cache_db_file_free((CACHE_FILE*) cache_file);
}

static void cache_db_file_free(CACHE_FILE *cache_file) {
	/*
	*  Free a CACHE_FILE instance.
	*/

	free(cache_file->fname);
	free(cache_file->fpath);
	free(cache_file);
}

static CACHE_FILE *cache_db_file_create(const CACHE *cache,
					const char *fname) {
	/*
	*  Create a CACHE_FILE instance. Returns a pointer to
	*  the new instance on success or a NULL pointer on failure.
	*/

	CACHE_FILE *n_cache_file = NULL;

	// Allocate memory for the CACHE_FILE instance.
	errno = 0;
	n_cache_file = malloc(sizeof(CACHE));
	if (n_cache_file == NULL) {
		printerrno("cache: malloc()");
		return NULL;
	}

	// Allocate the name string.
	errno = 0;
	n_cache_file->fname = calloc(strlen(fname) + 1, sizeof(char));
	if (n_cache_file->fname == NULL) {
		printerrno("cache: calloc()");
		free(n_cache_file);
		return NULL;
	}
	strcpy(n_cache_file->fname, fname);

	// Create the path string.
	errno = 0;
	n_cache_file->fpath = cache_get_path_to_file(cache, fname);
	if (n_cache_file->fpath == NULL) {
		printerr("Failed to get path to cache file.\n");
		free(n_cache_file->fname);
		free(n_cache_file);
		return NULL;
	}

	// Add the file timestamp.
	n_cache_file->tstamp = time(NULL);

	return n_cache_file;
}

int cache_db_file_unreg(CACHE *cache, const char *fname) {
	/*
	*  Unregister a cache file from 'cache'. Returns 0
	*  on success and 1 on failure.
	*/

	int index = -1;
	index = cache_db_file_get_index(cache, fname);

	if (index < 0) {
		printerr_va("Cache file '%s' not found.\n", fname);
		return 1;
	}

	if (!ptrarray_pop_ptr((PTRARRAY_TYPE(void)*) cache,
			cache->db->ptrs[index], 1)) {
		printerr("Failed to unregister cache file.\n");
		return 1;
	}
	return 0;
}

CACHE_FILE *cache_db_file_reg(CACHE *cache, const char *fname,
				unsigned int auto_rm) {
	/*
	*  Register the file 'fname' as a cache file for the
	*  cache 'cache'. Returns a pointer to the allocated
	*  CACHE_FILE instance on success or a NULL pointer
	*  on failure.
	*/

	int index = -1;
	int rm_index = 0;
	CACHE_FILE *n_cache_file = NULL;

	// Check if the supplied file is already registered.
	index = cache_db_file_get_index(cache, fname);
	if (index != -1) {
		printerr_va("Cache file %s already registered.\n", fname);
		return cache->db->ptrs[index];
	}

	/*
	*  Check if the cache directory has space for new files.
	*  If auto_rm is 1, the oldest file in the cache will
	*  be automatically removed in order to make space for the
	*  new one if needed.
	*/
	if (cache->db->ptrc >= cache->max_files) {
		printerr_va("Cache '%s' can't fit more files.\n", cache->name);
		if (auto_rm) {
			printverb("Removing cache files to make space for new ones.\n");
			rm_index = cache_db_file_get_index_oldest(cache);
			if (rm_index == -1) {
				printerr("Failed to get oldest file index.\n");
				return NULL;
			}
			if (cache_delete_file(cache, cache->db->ptrs[rm_index]->fname) != 0) {
				printerr("File deletion failed. Can't register file.\n");
				return NULL;
			}
		} else {
			return NULL;
		}
	}

	printverb_va("Registering file '%s' as a cache file.\n", fname);

	n_cache_file = cache_db_file_create(cache, fname);
	if (!n_cache_file) {
		printerr("Failed to create cache file instance.\n");
		return NULL;
	}

	if (!ptrarray_put_ptr((PTRARRAY_TYPE(void)*) cache->db,
				n_cache_file)) {
		printerr_va("Failed to register cache file '%s'.\n", fname);
		return NULL;
	}

	return n_cache_file;
}

static int cache_db_file_get_index_oldest(const CACHE *cache) {
	/*
	*  Get the index of the oldest file in the file db
	*  of 'cache' or -1 on failure.
	*/
	int tmp_index = -1;
	time_t tmp_tstamp = 0;

	if (cache->db->ptrc > 0) {
		tmp_tstamp = cache->db->ptrs[0]->tstamp;
		tmp_index = 0;
		for (size_t i = 0; i < cache->db->ptrc; i++) {
			if (cache->db->ptrs[i]->tstamp < tmp_tstamp) {
				tmp_tstamp = cache->db->ptrs[i]->tstamp;
				tmp_index = i;
			}
		}
	}
	return tmp_index;
}

static int cache_db_file_get_index(const CACHE *cache, const char *fname) {
	/*
	*  Return the index of the CACHE_FILE instance for 'fname' in
	*  the cache file database of 'cache'. If the file is not found,
	*  -1 is returned.
	*/
	for (size_t i = 0; i < cache->db->ptrc; i++) {
		if (strcmp(cache->db->ptrs[i]->fname, fname) == 0) {
			return i;
		}
	}
	return -1;
}

char *cache_get_path_to_file(const CACHE *cache, const char *fname) {
	/*
	*  Return a string pointer to the path to 'fname' or a
	*  NULL pointer on failure.
	*/
	return file_path_join(cache->path, fname);
}

int cache_has_file(const CACHE *cache, const char *fname) {
	/*
	*  Return 1 if 'cache' contains the file 'fname' and
	*  return 0 otherwise;
	*/
	if (cache_db_file_get_index(cache, fname) != -1) {
		return 1;
	}
	return 0;
}

int cache_delete_file(CACHE *cache, const char *fname) {
	/*
	*  Delete the file 'fname' from 'cache'.
	*  Returns 0 on success and 1 on failure.
	*/
	int index = -1;

	index = cache_db_file_get_index(cache, fname);
	if (index == -1) {
		printerr_va("File %s doesn't exist in cache %s.\n", fname, cache->name);
		return 1;
	}

	errno = 0;
	if (access(cache->db->ptrs[index]->fpath, F_OK) == 0) {
		errno = 0;
		if (unlink(cache->db->ptrs[index]->fpath) == -1) {
			printerrno("cache: unlink()");
			return 1;
		}
	} else {
		printerrno("cache: access()");
		return 1;
	}

	// Unregister the cache file.
	if (cache_db_file_unreg(cache, fname) != 0) {
		printerr("Failed to unregister cache file.\n");
		return 1;
	}
	return 0;
}

CACHE *cache_get_by_name(const char *name) {
	/*
	*  Get a cache by it's name. Returns a pointer to
	*  the CACHE instance if it's found and a NULL
	*  pointer otherwise.
	*/

	for (size_t i = 0; i < caches->ptrc; i++) {
		if (strcmp(caches->ptrs[i]->name, name) == 0) {
			return caches->ptrs[i];
		}
	}
	return NULL;
}

CACHE *cache_create(const char *cache_name) {
	/*
	*  Create a cache with the name 'cache_name'.
	*  Returns a pointer to a new CACHE instance on success
	*  or a NULL pointer on failure.
	*/

	CACHE *n_cache = NULL;

	printverb_va("Creating cache %s.\n", cache_name);

	// Allocate memory for the CACHE instance.
	errno = 0;
	n_cache = malloc(sizeof(CACHE));
	if (n_cache == NULL) {		printerrno("cache: malloc()");
		return NULL;
	}
	memset(n_cache, 0, sizeof(CACHE));

	// Copy the cache name.
	errno = 0;
	n_cache->name = calloc(strlen(cache_name) + 1, sizeof(char));
	if (n_cache->name == NULL) {
		printerrno("cache: calloc()");
		free(n_cache);
		return NULL;
	}
	memcpy(n_cache->name, cache_name, strlen(cache_name)*sizeof(char));

	// Get the full cache path.
	n_cache->path = file_path_join(cache_root, cache_name);
	if (n_cache->path == NULL) {
		free(n_cache->name);
		free(n_cache);
		return NULL;
	}

	// Create the cache directory.
	errno = 0;
	if (access(n_cache->path, F_OK) != 0) {
		if (errno == ENOENT) {
			errno = 0;
			if (mkdir(n_cache->path, CACHE_PERMISSIONS) == -1) {
				printerrno("cache: mkdir()");
				cache_destroy(n_cache, 0);
				return NULL;
			}
		} else {
			printerrno("cache: access()");
			cache_destroy(n_cache, 0);
			return NULL;
		}
	}

	// Set the default max_files value.
	n_cache->max_files = cache_default_max_files;

	// Setup the cache DB.
	n_cache->db = (PTRARRAY_TYPE(CACHE_FILE)*) ptrarray_create(&cache_db_file_free_wrapper);
	if (!n_cache->db) {
		cache_destroy(n_cache, 0);
		return NULL;
	}

	// Add the cache pointer to the caches array.
	if (!ptrarray_put_ptr((PTRARRAY_TYPE(void)*) caches, n_cache)) {
		printerr("Failed to add CACHE pointer to PTRARRAY.\n");
		cache_destroy(n_cache, 0);
		return NULL;
	}

	return n_cache;
}

void cache_destroy(CACHE *cache, int del_files) {
	/*
	*  Destroy a cache and free the memory allocated to it.
	*  If del_files is 0, the cache directory is left in place.
	*  Otherwise the cache directory is deleted too.
	*/

	if (cache != NULL) {
		if (del_files) {
			// Delete the cache directory recursively.
			errno = 0;
			if (access(cache->path, F_OK) == 0) {
				if (rmdir_recursive(cache->path) != 0) {
					printerr("Failed to delete cache.\n");
				}
			} else {
				printerrno("cache: access()");
			}
		}

		// Free the cache file database.
		ptrarray_free_ptrs((PTRARRAY_TYPE(void)*) cache->db);
		ptrarray_free((PTRARRAY_TYPE(void)*) cache->db);
		cache->db = NULL;

		// Free the cache instance.
		free(cache->name);
		free(cache->path);
		free(cache);
	}
}

int cache_setup(void) {
	/*
	*  Caching system setup function. This function must be run
	*  before running any of the other functions in this file.
	*/
	printverb("Cache setup.\n");

	cache_root = config_get_str_param("cache_root");
	if (!cache_root) {
		cache_root = NULL;
		return 1;
	}

	cache_default_max_files = config_get_lint_param("cache_default_max_files");
	if (cache_default_max_files <= 0) {
		cache_default_max_files = 0;
		return 1;
	}

	// Create the cache root if it doesn't exist.
	errno = 0;
	if (access(cache_root, F_OK) != 0) {
		if (errno == ENOENT) {
			errno = 0;
			if (mkdir(cache_root, CACHE_PERMISSIONS) == -1) {
				printerrno("cache: mkdir()");
				return 1;
			}
		} else {
			printerrno("cache: access()");
			return 1;
		}
	}

	// Setup the caches PTRARRAY.
	caches = (PTRARRAY_TYPE(CACHE)*) ptrarray_create(NULL);
	if (!caches) {
		return 1;
	}

	return 0;
}

void cache_cleanup(int del_files) {
	/*
	*  Caching system cleanup function. If del_files is 0, the cache
	*  directories will be left in place. Otherwise the cache
	*  directories will be deleted.
	*/

	// Destroy all existing caches.
	if (caches) {
		printverb("Cache cleanup.\n");
		if (!del_files) {
			printverb("Leaving cache files in place.\n");
		}
		for (size_t i = 0; i < caches->ptrc; i++) {
			cache_destroy(caches->ptrs[i], del_files);
			caches->ptrs[i] = NULL;
		}
		ptrarray_free((PTRARRAY_TYPE(void)*) caches);
	}
}
