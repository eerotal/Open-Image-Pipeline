#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>
#include <math.h>
#include <errno.h>
#include <unistd.h>

#define CACHE_DIR "plugins/cache/"
#define CACHE_PERMISSIONS S_IRWXU

char *cache_create(const char *name, unsigned int id) {
	// Construct full cache path.
	unsigned int id_str_len = 0;
	char *id_str = NULL;
	char *path = NULL;

	if (id == 0) {
		id_str_len = 2;
	} else {
		id_str_len = floor(log10(id)) + 2;
	}

	// Allocate memory for the ID string.
	id_str = calloc(id_str_len, sizeof(char));
	if (id_str == NULL) {
		fprintf(stderr, "calloc(): Failed to allocate memory.\n");
		return NULL;
	}
	sprintf(id_str, "%i", id);

	/*
	*  Allocate memory for the whole path string.
	*  The string is of the form "<Cache Dir>/<plugin>-<ID>".
	*/
	path = calloc(strlen(CACHE_DIR) + strlen(name) + 1 + id_str_len + 1, sizeof(char));
	if (path == NULL) {
		fprintf(stderr, "calloc(): Failed to allocate memory.\n");
		free(id_str);
		return NULL;
	}
	strcat(path, CACHE_DIR);
	strcat(path, name);
	strcat(path, "-");
	strcat(path, id_str);
	free(id_str);

	printf("cache: Creating cache directory: %s\n", path);

	if (access(path, F_OK) != 0) {
		if (errno == ENOENT) {
			// Create directory.
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
	// TODO: Implement deleting all the cache files left behind by plugins.
	printf("cache: Deleting all cache files. NOT IMPLEMENTED YET\n");
	return 0;
}
