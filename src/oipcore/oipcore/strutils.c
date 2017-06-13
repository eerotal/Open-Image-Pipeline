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

#define PRINT_IDENTIFIER "strutils"


#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <stdarg.h>

#include "oipcore/strutils.h"
#include "oipcore/abi/output.h"

char *strutils_cat(size_t n, char *sep, ...) {
	/*
	*  Concatenate strings from the va_args with the string
	*  'sep' between each one. Returns a pointer to the new
	*  string on success or a NULL pointer on failure.
	*/
	size_t ret_size = 0;
	size_t ret_size_old = 0;
	char *ret = NULL;
	char *tmp_ret = NULL;
	char *tmp_part = NULL;
	va_list va;

	va_start(va, sep);

	for (size_t i = 0; i < n; i++) {
		tmp_part = va_arg(va, char*);
		if (tmp_part) {
			ret_size_old = ret_size;
			if (i == 0) {
				ret_size = strlen(tmp_part) + 1;
			} else {
				ret_size += strlen(sep) + strlen(tmp_part);
			}

			errno = 0;
			tmp_ret = realloc(ret, ret_size*sizeof(char));
			if (!tmp_ret) {
				printerrno("realloc()");
				free(ret);
				va_end(va);
				return NULL;
			}
			ret = tmp_ret;

			// Set the newly allocated part of ret to all zeroes.
			memset(ret + ret_size_old, 0, ret_size - ret_size_old);

			if (i != 0) {
				strcat(ret, sep);
			}
			strcat(ret, tmp_part);
		}
	}
	va_end(va);
	return ret;
}
