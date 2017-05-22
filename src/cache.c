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

#define CACHE_DIR "plugins/cache/"
#define CACHE_PERMISSIONS S_IRWXU

char *cache_get_dir(void) {
	return CACHE_DIR;
}

char *cache_create(const char *name) {
	// Construct full cache path.
	char *path = NULL;

	/*
	*  Allocate memory for the whole path string.
	*  The string is of the form "<Cache Dir>/<Cache Name>".
	*/
	errno = 0;
	path = calloc(strlen(CACHE_DIR) + strlen(name) + 1, sizeof(char));
	if (path == NULL) {
		perror("calloc(): ");
		return NULL;
	}
	strcat(path, CACHE_DIR);
	strcat(path, name);

	printf("cache: Creating cache directory: %s\n", path);

	errno = 0;
	if (access(CACHE_DIR, F_OK) != 0) {
		if (errno == ENOENT) {
			// Create the cache directory.
			if (mkdir(CACHE_DIR, CACHE_PERMISSIONS) == -1) {
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

int cache_delete_all(void) {
	printf("cache: Deleting all cache files.\n");
	if (rmdir_recursive(CACHE_DIR) == 1) {
		printf("Recursive cache delete failed.\n");
		return 1;
	}
	return 0;
}
