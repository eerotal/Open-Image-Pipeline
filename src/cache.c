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

#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>
#include <math.h>
#include <errno.h>
#include <unistd.h>

#include "file.h"
#include "cache_priv.h"
#include "cli_priv.h"

#define CACHE_ROOT "plugins/cache/"
#define CACHE_PERMISSIONS S_IRWXU

static CACHE **caches = NULL;
static unsigned int caches_count = 0;

int cache_has_file(CACHE *cache, const char *fname) {
	/*
	*  Check if a cache contains a file. Returns 1
	*  if it does, 0 if it doesn't and -1 on error.
	*/

	int ret = 0;
	char *fpath = file_path_join(cache->path, fname);
	if (fpath == NULL) {
		return -1;
	}

	errno = 0;
	if (access(fpath, F_OK) == 0) {
		ret = 1;
	} else {
		if (errno == ENOENT) {
			ret = 0;
		} else {
			ret = -1;
		}
	}
	free(fpath);
	return ret;
}

char *cache_get_path_to_file(CACHE *cache, const char *fname) {
	/*
	*  Get the path to the file named 'fname' in the cache 'cache'.
	*  Returns a pointer to a newly allocated string on success and
	*  a NULL pointer on failure.
	*/
	return file_path_join(cache->path, fname);
}

CACHE *cache_create(const char *cache_name) {
	/*
	*  Create a cache with the name 'cache_name'.
	*  Returns a pointer to a new CACHE instance on success
	*  and a NULL pointer on failure.
	*/

	CACHE *n_cache = NULL;
	CACHE **tmp_caches = NULL;

	printf("cache: Creating cache %s.\n", cache_name);

	// Allocate memory for the CACHE instance.
	errno = 0;
	n_cache = malloc(sizeof(CACHE));
	if (n_cache == NULL) {
		perror("cache: malloc()");
		return NULL;
	}
	memset(n_cache, 0, sizeof(CACHE));

	// Copy the cache name.
	errno = 0;
	n_cache->name = calloc(strlen(cache_name) + 1, sizeof(char));
	if (n_cache->name == NULL) {
		perror("cache: calloc()");
		free(n_cache);
		return NULL;
	}
	memcpy(n_cache->name, cache_name, strlen(cache_name)*sizeof(char));

	// Get the full cache path.
	n_cache->path = file_path_join(CACHE_ROOT, cache_name);
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
				perror("cache: mkdir()");
				cache_destroy(n_cache, 0);
				return NULL;
			}
		} else {
			perror("cache: access()");
			cache_destroy(n_cache, 0);
			return NULL;
		}
	}

	// Add the cache pointer to the caches array.
	caches_count++;
	errno = 0;
	tmp_caches = realloc(caches, caches_count*sizeof(CACHE*));
	if (tmp_caches == NULL) {
		perror("cache: realloc()");
		caches_count--;
		cache_destroy(n_cache, 1);
		return NULL;
	}
	caches = tmp_caches;
	caches[caches_count - 1] = n_cache;

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
					printf("cache: Failed to delete cache.\n");
				}
			} else {
				perror("cache: access()");
			}
		}

		// Free allocated memory.
		if (cache->name != NULL) {
			free(cache->name);
		}
		if (cache->path != NULL) {
			free(cache->path);
		}
		free(cache);
	}
}

int cache_setup(void) {
	/*
	*  Caching system setup function. This function must be run
	*  before running any of the other functions in this file.
	*/
	printf("cache: Cache setup.\n");

	// Create the cache root if it doesn't exist.
	errno = 0;
	if (access(CACHE_ROOT, F_OK) != 0) {
		if (errno == ENOENT) {
			errno = 0;
			if (mkdir(CACHE_ROOT, CACHE_PERMISSIONS) == -1) {
				perror("cache: mkdir()");
				return 1;
			}
		} else {
			perror("cache: access()");
			return 1;
		}
	}
	return 0;
}

void cache_cleanup(int del_files) {
	/*
	*  Caching system cleanup function. If del_files is 0, the cache
	*  directories will be left in place. Otherwise the cache
	*  directories will be deleted.
	*/
	printf("cache: Cache cleanup.\n");

	// Destroy all existing caches.
	if (caches != NULL) {
		if (!del_files) {
			printf("cache: Leaving cache files in place.\n");
		}
		for (unsigned int i = 0; i < caches_count; i++) {
			cache_destroy(caches[i], del_files);
		}
		free(caches);
	}
}
