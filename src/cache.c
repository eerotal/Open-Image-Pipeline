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
#include <time.h>

#include "file.h"
#include "cache_priv.h"

#define CACHE_ROOT "plugins/cache/"
#define CACHE_PERMISSIONS S_IRWXU
#define CACHE_DEFAULT_MAX_FILES 20

static CACHE **caches = NULL;
static unsigned int caches_count = 0;

static int cache_db_shrink(CACHE *cache);
static int cache_db_get_file_index(CACHE *cache, const char *fname);

void cache_dump(CACHE *cache) {
	/*
	*  Dump info about 'cache' to STDOUT.
	*/
	printf("Cache '%s':\n", cache->name);
	printf("  Name:      %s\n", cache->name);
	printf("  Path:      %s\n", cache->path);
	printf("  Max files: %u\n", cache->max_files);
	printf("  Files:\n");
	for (unsigned int i = 0; i < cache->db_len; i++) {
		printf("    %s : %s\n", cache->db[i]->fname, cache->db[i]->fpath);
	}
}

void cache_dump_all(void) {
	/*
	*  Dump info about all caches to STDOUT.
	*/
	for (unsigned int i = 0; i < caches_count; i++) {
		cache_dump(caches[i]);
	}
}

int cache_db_unreg_file(CACHE *cache, const char *fname) {
	/*
	*  Unregister a cache file for 'cache'. Returns 0 if
	*  the file is successfully unregistered and 1 if it
	*  doesn't exist.
	*/
	int index = -1;
	index = cache_db_get_file_index(cache, fname);

	if (index == -1) {
		printf("cache: Cache doesn't have file %s.\n", fname);
		return 1;
	}

	// Free allocated resources.
	free(cache->db[index]->fname);
	free(cache->db[index]->fpath);
	free(cache->db[index]);
	cache->db[index] = NULL;

	if (cache_db_shrink(cache) != 0) {
		return 1;
	}
	return 0;
}

CACHE_FILE *cache_db_reg_file(CACHE *cache, const char *fname) {
	/*
	*  Register a file with the caching system. When a file is
	*  written to a cache directory, this function must be called.
	*  If this function is not called, the file won't be treated
	*  as a cache file and it won't get deleted automatically for
	*  example. Returns a pointer to the new CACHE_FILE instance on
	*  success and a NULL pointer on failure.
	*/

	int index = 0;
	CACHE_FILE *n_cache_file = NULL;
	CACHE_FILE **tmp_cache_db = NULL;

	// Check if the supplied file is already registered.
	index = cache_db_get_file_index(cache, fname);
	if (index != -1) {
		printf("cache: Cache file %s already registered.\n", fname);
		return cache->db[index];
	}

	// Allocate memory for the CACHE_FILE instance.
	errno = 0;
	n_cache_file = malloc(sizeof(CACHE));
	if (n_cache_file == NULL) {
		perror("cache: malloc()");
		return NULL;
	}

	// Allocate the name string.
	errno = 0;
	n_cache_file->fname = calloc(strlen(fname) + 1, sizeof(char));
	if (n_cache_file->fname == NULL) {
		perror("cache: calloc()");
		free(n_cache_file);
		return NULL;
	}
	strcpy(n_cache_file->fname, fname);

	// Create the path string.
	errno = 0;
	n_cache_file->fpath = cache_get_path_to_file(cache, fname);
	if (n_cache_file->fpath == NULL) {
		printf("cache: Failed to get path to cache file.\n");
		free(n_cache_file->fname);
		free(n_cache_file);
		return NULL;
	}

	// Add the file timestamp.
	n_cache_file->tstamp = time(NULL);

	// Add the CACHE_FILE instance to the cache DB.
	cache->db_len++;
	tmp_cache_db = realloc(cache->db, cache->db_len*sizeof(CACHE_FILE*));
	if (tmp_cache_db == NULL) {
		perror("cache: realloc()");
		cache->db_len--;
		free(n_cache_file->fname);
		free(n_cache_file->fpath);
		free(n_cache_file);
		return NULL;
	}
	cache->db = tmp_cache_db;

	cache->db[cache->db_len - 1] = n_cache_file;

	return n_cache_file;
}

static int cache_db_shrink(CACHE *cache) {
	/*
	*  Shrink the cache db by removing any NULL pointers from it.
	*  Returns 0 on success and 1 on failure.
	*/
	unsigned int n_db_len = 0;
	CACHE_FILE **n_db = NULL;
	CACHE_FILE **tmp_db = NULL;

	for (unsigned int i = 0; i < cache->db_len; i++) {
		if (cache->db[i] != NULL) {
			n_db_len++;
			errno = 0;
			tmp_db = realloc(n_db, n_db_len*sizeof(CACHE_FILE*));
			if (tmp_db == NULL) {
				perror("cache: realloc()");
				free(n_db);
				return 1;
			}
			n_db = tmp_db;
			n_db[n_db_len - 1] = cache->db[i];
		}
	}

	free(cache->db);
	cache->db = n_db;
	cache->db_len = n_db_len;
	return 0;
}

static int cache_db_get_file_index(CACHE *cache, const char *fname) {
	/*
	*  Return the index of the CACHE_FILE instance for 'fname' in
	*  the cache file database of 'cache'. If the file is not found,
	*  -1 is returned.
	*/
	for (unsigned int i = 0; i < cache->db_len; i++) {
		if (strcmp(cache->db[i]->fname, fname) == 0) {
			return i;
		}
	}
	return -1;
}

char *cache_get_path_to_file(CACHE *cache, const char *fname) {
	/*
	*  Return a string pointer to the path to 'fname' or a
	*  NULL pointer on failure.
	*/
	return file_path_join(cache->path, fname);
}

int cache_has_file(CACHE *cache, const char *fname) {
	/*
	*  Return 1 if 'cache' contains the file 'fname' and
	*  return 0 otherwise;
	*/
	if (cache_db_get_file_index(cache, fname) != -1) {
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

	index = cache_db_get_file_index(cache, fname);
	if (index == -1) {
		printf("cache: File %s doesn't exist in cache %s.\n", fname, cache->name);
		return 1;
	}

	errno = 0;
	if (access(cache->db[index]->fpath, F_OK) == 0) {
		errno = 0;
		if (unlink(cache->db[index]->fpath) == -1) {
			perror("cache: unlink()");
			return 1;
		}
	} else {
		perror("cache: access()");
		return 1;
	}

	// Unregister the cache file.
	if (cache_db_unreg_file(cache, fname) != 0) {
		printf("cache: Failed to unregister cache file.\n");
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

	for (unsigned int i = 0; i < caches_count; i++) {
		if (strcmp(caches[i]->name, name) == 0) {
			return caches[i];
		}
	}
	return NULL;
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

	// Set the default max_files value.
	n_cache->max_files = CACHE_DEFAULT_MAX_FILES;

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

		// Free the cache file database.
		for (unsigned int i = 0; i < cache->db_len; i++) {
			free(cache->db[i]->fname);
			free(cache->db[i]->fpath);
			free(cache->db);
			cache->db = NULL;
		}

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
