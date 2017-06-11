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

#ifndef INCLUDED_PTRARRAY
	#define INCLUDED_PTRARRAY

	#include <stdio.h>

	/*
	*  Macro for defining a PTRARRAY_[type] type.
	*  'type' should be a valid C type.
	*/
	#define PTRARRAY_TYPE_DEF(type)			\
	typedef struct STRUCT_PTRARRAY_##type {		\
		type **ptrs;				\
		size_t ptrc;				\
		void (*free_func)(void*);		\
	} PTRARRAY_##type				\

	/*
	*  A macro that expands to a PTRARRAY_[type]. This
	*  can be used for type casting etc.
	*/
	#define PTRARRAY_TYPE(type) PTRARRAY_##type

	// Define the PTRARRAY_void type.
	PTRARRAY_TYPE_DEF(void);

	PTRARRAY_TYPE(void) *ptrarray_create(void (*free_func)(void*));
	int ptrarray_free_ptrs(PTRARRAY_TYPE(void) *ptrarray);
	void ptrarray_free(PTRARRAY_TYPE(void) *ptrarray);

	PTRARRAY_TYPE(void) *ptrarray_realloc(PTRARRAY_TYPE(void) *ptrarray,
						size_t ptrc);
	PTRARRAY_TYPE(void) *ptrarray_put_ptr(PTRARRAY_TYPE(void) *ptrarray,
						void *ptr);
	PTRARRAY_TYPE(void) *ptrarray_put_data(PTRARRAY_TYPE(void) *ptrarray,
						void *data, size_t data_size);
	PTRARRAY_TYPE(void) *ptrarray_pop_ptr(PTRARRAY_TYPE(void) *ptrarray,
						void *ptr, int free_ptr);
	PTRARRAY_TYPE(void) *ptrarray_shrink(PTRARRAY_TYPE(void) *ptrarray);

#endif

