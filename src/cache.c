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
#include <dirent.h>

#include "file.h"

#define CACHE_ROOT "plugins/cache/"
#define CACHE_PERMISSIONS S_IRWXU

char *cache_get_root(void) {
	return CACHE_ROOT;
}

char *cache_get_path(const char *cache_name) {
	/*
	*  Get the full path to a named cache.
	*  Returns a pointer to a string containing the
	*  path on success and NULL on failure.
	*/

	char *fullpath = NULL;
	errno = 0;
	fullpath = calloc(strlen(CACHE_ROOT) + strlen(cache_name) + 1, sizeof(char));
	if (fullpath == NULL) {
		perror("calloc(): ");
		return NULL;
	}
	strcat(fullpath, CACHE_ROOT);
	strcat(fullpath, cache_name);
	return fullpath;
}

char *cache_get_file_path(const char *cache_name, const char *cache_id) {
	/*
	*  Get the path to the file 'cache_id' in a named cache.
	*  Returns a pointer to a string containing the path or
	*  NULL on failure.
	*/
	char *cache_path = NULL;
	char *cache_file = NULL;
	cache_path = cache_get_path(cache_name);
	if (cache_path == NULL) {
		return NULL;
	}

	cache_file = file_path_join(cache_path, cache_id);
	if (cache_file == NULL) {
		free(cache_path);
		return NULL;
	}
	return cache_file;
}

char *cache_create(const char *cache_name) {
	/*
	*  Create a named cache.
	*  Returns a pointer to a string containing the
	*  cache path on success and NULL on failure.
	*/

	char *path = NULL;
	path = cache_get_path(cache_name);
	if (path == NULL) {
		return NULL;
	}

	printf("cache: Creating cache directory: %s\n", path);

	errno = 0;
	if (access(CACHE_ROOT, F_OK) != 0) {
		if (errno == ENOENT) {
			// Create the cache directory.
			if (mkdir(CACHE_ROOT, CACHE_PERMISSIONS) == -1) {
				perror("mkdir(): ");
				free(path);
				return NULL;
			}
		} else {
			// Error occured.
			perror("access(): ");
			free(path);
			return NULL;
		}
	}

	errno = 0;
	if (access(path, F_OK) != 0) {
		if (errno == ENOENT) {
			// Create the cache subdirectory.
			if (mkdir(path, CACHE_PERMISSIONS) == -1) {
				perror("mkdir(): ");
				free(path);
				return NULL;
			}
			return path;
		} else {
			// An error occured.
			perror("access(): ");
			free(path);
			return NULL;
		}
	} else {
		// Cache exists.
		printf("cache: Cache already exists.\n");
		return path;
	}
	free(path);
	return NULL;
}

int cache_file_exists(const char *cache_name, const char *cache_id) {
	/*
	*  Check if the file cache_id exists in cache_name.
	*  Returns 1 in case it does and 0 otherwise.
	*/
	char *cache_file_path = cache_get_file_path(cache_name, cache_id);
	if (access(cache_file_path, F_OK) == 0) {
		free(cache_file_path);
		return 1;
	}
	free(cache_file_path);
	return 0;
}

int cache_delete_all(void) {
	printf("cache: Deleting all cache files.\n");
	if (rmdir_recursive(CACHE_ROOT) == 1) {
		printf("Recursive cache delete failed.\n");
		return 1;
	}
	return 0;
}
