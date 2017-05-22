#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>

#define DIRECTORY_SEPARATOR '/'

char *path_ensure_trailing_slash(const char *path) {
	/*
	*  Make sure the supplied path has a trailing slash and if
	*  it doesn't, add one. This function returns a pointer to
	*  a newly allocated string containing the result.
	*  A NULL pointer is returned if an error occurs.
	*/
	char *path_proper = NULL;

	errno = 0;
	if (path[strlen(path) - 1] == DIRECTORY_SEPARATOR) {
		path_proper = calloc(strlen(path) + 1, sizeof(char));
		if (path_proper == NULL) {
			perror("calloc(): ");
			return NULL;
		}
		strcpy(path_proper, path);
	} else {
		path_proper = calloc(strlen(path) + 2, sizeof(char));
		if (path_proper == NULL) {
			perror("calloc(): ");
			return NULL;
		}
		strcat(path_proper, path);
		path_proper[strlen(path)] = '/';
	}
	return path_proper;
}

char *file_path_join(const char *s1, const char *s2) {
	/*
	*  Join two strings into a filepath. This function
	*  returns a pointer to a new string on success and
	*  NULL on failure. Note that this function will fail
	*  if either one of the arguments are NULL.
	*/

	char *ret = NULL;
	char *s1_proper = NULL;
	if (s1 == NULL || s2 == NULL) {
		return NULL;
	}

	s1_proper = path_ensure_trailing_slash(s1);
	if (s1_proper == NULL) {
		return NULL;
	}

	ret = calloc(strlen(s1_proper) + strlen(s2) + 1, sizeof(char));
	if (ret == NULL) {
		perror("calloc(): ");
		free(s1_proper);
		return NULL;
	}

	strcat(ret, s1_proper);
	strcat(ret, s2);
	free(s1_proper);

	return ret;
}

int rmdir_recursive(const char *rpath) {
	struct stat statbuf;
	struct dirent *f = NULL;
	DIR *dir = NULL;
	char *fpath = NULL;
	char *tmp_fpath = NULL;
	char *rpath_proper = NULL;

	rpath_proper = path_ensure_trailing_slash(rpath);
	if (rpath_proper == NULL) {
		return 1;
	}

	errno = 0;
	dir = opendir(rpath_proper);
	if (dir == NULL) {
		perror("opendir(): ");
		return 1;
	}

	errno = 0;
	while ((f = readdir(dir)) != NULL) {
		// Exclude . and ..
		if (strcmp(f->d_name, "..") == 0 || strcmp(f->d_name, ".") == 0) {
			continue;
		}

		// Construct the path name for the current file.
		errno = 0;
		tmp_fpath = realloc(fpath, strlen(rpath_proper) + strlen(f->d_name) + 1);
		if (tmp_fpath == NULL) {
			perror("realloc(): ");
			free(rpath_proper);
			closedir(dir);
			return 1;
		}
		fpath = tmp_fpath;
		memset(fpath, 0, (strlen(fpath) + 1)*sizeof(char));
		strcat(fpath, rpath_proper);
		strcat(fpath, f->d_name);

		// Get information about the path.
		errno = 0;
		if (stat(fpath, &statbuf) == -1) {
			perror("stat(): ");
			free(fpath);
			free(rpath_proper);
			closedir(dir);
			return 1;
		}

		/*
		*  Check if the path is a directory and
		*  if it is, run this function on that
		*  one too. Otherwise unlink the file.
		*/
		if (S_ISDIR(statbuf.st_mode)) {
			rmdir_recursive(fpath);
		} else {
			if (unlink(fpath) == -1) {
				perror("unlink(): ");
				free(fpath);
				free(rpath_proper);
				closedir(dir);
				return 1;
			}
		}
	}

	if (errno != 0) {
		perror("readdir(): ");
		free(fpath);
		free(rpath_proper);
		closedir(dir);
		return 1;
	}

	free(fpath);
	free(rpath_proper);
	closedir(dir);

	// Remove the (now empty) directory.
	errno = 0;
	if (rmdir(rpath) == -1) {
		perror("rmdir(): ");
		return 1;
	}

	return 0;
}
