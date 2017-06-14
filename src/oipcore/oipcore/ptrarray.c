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

#define PRINT_IDENTIFIER "ptrarray"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#include "oipcore/abi/output.h"
#include "oipcore/ptrarray.h"

PTRARRAY_TYPE(void) *ptrarray_create(void (*const free_func)(void*)) {
	/*
	*  Create a new PTRARRAY instance. free_func is the
	*  function to call when freeing pointers in the PTRARRAY.
	*  If pointer freeing is not needed, free_func can be
	*  set to NULL.
	*/
	PTRARRAY_TYPE(void) *ret = NULL;
	errno = 0;
	ret = calloc(1, sizeof(PTRARRAY_TYPE(void)));
	if (!ret) {
		printerrno("calloc()");
		return NULL;
	}
	ret->free_func = free_func;
	return ret;
}

PTRARRAY_TYPE(void) *ptrarray_realloc(PTRARRAY_TYPE(void) *ptrarray,
						const size_t ptrc) {
	/*
	*  Reallocate the internal pointer array in the PTRARRAY
	*  instance. Returns a pointer to the PTRARRAY instance on
	*  success or a NULL pointer on failure. On failure the
	*  contents of the original PTRARRAY instance are not modified.
	*/
	void **tmp_ptrs = NULL;

	errno = 0;
	tmp_ptrs = realloc(ptrarray->ptrs, ptrc*sizeof(void*));
	if (!tmp_ptrs) {
		printerrno("realloc()");
		return NULL;
	}
	ptrarray->ptrs = tmp_ptrs;
	ptrarray->ptrc = ptrc;

	return ptrarray;
}

void *ptrarray_put_ptr(PTRARRAY_TYPE(void) *ptrarray, void *ptr) {
	/*
	*  Add a pointer to the PTRARRAY instance. Returns a the added
	*  pointer on success or a NULL pointer on failure. On failure
	*  the contents of the original PTRARRAY instance are not modified.
	*/
	PTRARRAY_TYPE(void) *tmp = NULL;

	tmp = ptrarray_realloc(ptrarray, ptrarray->ptrc + 1);
	if (!tmp) {
		return NULL;
	}
	tmp->ptrs[ptrarray->ptrc - 1] = ptr;
	return ptr;
}

void *ptrarray_put_data(PTRARRAY_TYPE(void) *ptrarray,
			void *data, const size_t data_size) {
	/*
	*  Copy 'data_size' number of bytes from 'data' to a new
	*  memory location and add the pointer to that location
	*  into the PTRARRAY instance. Returns the allocated pointer
	*  on success or a NULL pointer on failure. On failure
	*  the contents of the original PTRARRAY instance are not
	*  modified.
	*/
	void *tmp_ptr = NULL;
	tmp_ptr = malloc(data_size);
	if (!tmp_ptr) {
		return NULL;
	}

	memcpy(tmp_ptr, data, data_size);
	if (!ptrarray_put_ptr(ptrarray, tmp_ptr)) {
		free(tmp_ptr);
		return NULL;
	}
	return tmp_ptr;
}

PTRARRAY_TYPE(void) *ptrarray_shrink(PTRARRAY_TYPE(void) *ptrarray) {
	/*
	*  Remove all NULL pointers from the PTRARRAY. Returns
	*  a pointer to a new PTRARRAY on success or a NULL pointer
	*  on failure. On failure the contents of the original PTRARRAY
	*  instance are not modified.
	*/
	PTRARRAY_TYPE(void) *ret = NULL;
	ret = ptrarray_create(ptrarray->free_func);
	if (!ret) {
		return NULL;
	}

	for (size_t i = 0; i < ptrarray->ptrc; i++) {
		if (ptrarray->ptrs[i]) {
			if (!ptrarray_put_ptr(ret, ptrarray->ptrs[i])) {
				ptrarray_free((PTRARRAY_TYPE(void)*) ret);
				return NULL;
			}
			ret->ptrc++;
		}
	}
	ptrarray_free((PTRARRAY_TYPE(void)*) ptrarray);
	return ret;
}

PTRARRAY_TYPE(void) *ptrarray_pop_ptr(PTRARRAY_TYPE(void) *ptrarray,
					void *ptr, const int free_ptr) {
	/*
	*  Pop a pointer from a PTRARRAY. Returns a new PTRARRAY
	*  pointer on success or a NULL pointer on failure. On failure
	*  the contents of the original PTRARRAY instance are not
	*  modified.
	*/
	PTRARRAY_TYPE(void) *ret = NULL;
	int i = ptrarray_get_ptr_index(ptrarray, ptr);
	if (i < 0) {
		return NULL;
	}

	ptrarray->ptrs[i] = NULL;
	ret = ptrarray_shrink(ptrarray);
	if (!ret) {
		// Reset the pointer back to original.
		ptrarray->ptrs[i] = ptr;
		return NULL;
	}
	if (free_ptr) {
		ptrarray->free_func(ptr);
	}
	return ret;
}

int ptrarray_get_ptr_index(PTRARRAY_TYPE(void) *ptrarray, const void *ptr) {
	/*
	*  Get the index of ptr in the ptrarray or -1 if the
	*  ptrarray doesn't contain ptr.
	*/
	for (size_t i = 0; i < ptrarray->ptrc; i++) {
		if (ptrarray->ptrs[i] == ptr) {
			return i;
		}
	}
	return -1;
}

void ptrarray_free(PTRARRAY_TYPE(void) *ptrarray) {
	/*
	*  Free the PTRARRAY instance.
	*/
	if (ptrarray->ptrs) {
		free(ptrarray->ptrs);
	}
	free(ptrarray);
}

int ptrarray_free_ptrs(PTRARRAY_TYPE(void) *ptrarray) {
	/*
	*  Free all the pointers in the PTRARRAY instance using
	*  the free_func function pointer specified by the user.
	*  This function won't free pointers multiple times
	*  even if the same pointer is in the PTRARRAY instance
	*  more than once. This function also won't attempt to
	*  free NULL pointers, even though a PTRARRAY is actually
	*  in an undefined state if it contains NULL pointers.
	*  Returns 0 on success and 1 on failure.
	*/
	if (!ptrarray->free_func) {
		printerr("No freeing function specified.\n");
		return 1;
	}
	for (size_t a = 0; a < ptrarray->ptrc; a++) {
		if (!ptrarray->ptrs[a]) {
			continue;
		}
		for (size_t b = 0; b < ptrarray->ptrc; b++) {
			if (a == b) {
				continue;
			} else if (ptrarray->ptrs[a] == ptrarray->ptrs[b]) {
				ptrarray->ptrs[b] = NULL;
			}
		}
		ptrarray->free_func(ptrarray->ptrs[a]);
	}
	free(ptrarray->ptrs);
	ptrarray->ptrs = NULL;
	ptrarray->ptrc = 0;
	return 0;
}
