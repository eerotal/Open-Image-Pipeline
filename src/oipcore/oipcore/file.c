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

#define PRINT_IDENTIFIER "file"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdarg.h>

#include "oipcore/abi/output.h"
#include "oipcore/strutils.h"

#define DIRECTORY_SEPARATOR '/'

char *file_path_join(size_t n, ...) {
	/*
	*  Join 'n' number of path components from the variadic
	*  argument list into a filepath. Returns a pointer to
	*  a new string on success or a NULL pointer on failure.
	*/

	char *tmp = NULL;
	char *ret = NULL;
	va_list va;

	va_start(va, n);
	tmp = strutils_cat_va(n, "/", &va);
	if (!tmp) {
		return NULL;
	}
	ret = strutils_strip_subseq(tmp, DIRECTORY_SEPARATOR);
	if (!ret) {
		free(tmp);
		return NULL;
	}
	va_end(va);
	return ret;
}

int file_rmdir_recursive(const char *rpath) {
	/*
	*  Recursively remove a directory and it's contents. The
	*  directory path is specified in 'rpath'. Returns 0 on
	*  success and 1 on failure.
	*/
	struct stat statbuf;
	struct dirent *f = NULL;
	DIR *dir = NULL;
	char *fpath = NULL;

	errno = 0;
	if (access(rpath, F_OK) != 0) {
		printerrno("file: access()");
		return 1;
	}

	errno = 0;
	dir = opendir(rpath);
	if (!dir) {
		printerrno("file: opendir()");
		return 1;
	}

	errno = 0;
	while ((f = readdir(dir))) {
		// Exclude . and ..
		if (strcmp(f->d_name, "..") == 0 || strcmp(f->d_name, ".") == 0) {
			continue;
		}

		// Construct the path name for the current file.
		fpath = strutils_cat(2, "/", rpath, f->d_name);
		if (!fpath) {
			printerr("Failed to construct filepath for unlink.\n");
			closedir(dir);
			return 1;
		}

		// Get information about the path.
		errno = 0;
		if (stat(fpath, &statbuf) == -1) {
			printerrno("stat()");
			free(fpath);
			closedir(dir);
			return 1;
		}

		/*
		*  Check if the path is a directory and
		*  if it is, run this function on that
		*  one too. Otherwise unlink the file.
		*/
		if (S_ISDIR(statbuf.st_mode)) {
			file_rmdir_recursive(fpath);
		} else {
			if (unlink(fpath) == -1) {
				printerrno("unlink()");
				free(fpath);
				closedir(dir);
				return 1;
			}
		}
	}

	if (errno != 0) {
		printerrno("readdir()");
		free(fpath);
		closedir(dir);
		return 1;
	}

	free(fpath);
	closedir(dir);

	// Remove the (now empty) directory.
	errno = 0;
	if (rmdir(rpath) == -1) {
		printerrno("rmdir()");
		return 1;
	}

	return 0;
}
