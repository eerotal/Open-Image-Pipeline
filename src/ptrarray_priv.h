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

#ifndef INCLUDED_PTRARRAY_PRIV
	#define INCLUDED_PTRARRAY_PRIV

	#include <stdio.h>

	typedef struct STRUCT_PTRARRAY {
		void **ptrs;
		size_t ptrc;
	} PTRARRAY;

	PTRARRAY *ptrarray_create(void);
	PTRARRAY *ptrarray_realloc(PTRARRAY *ptrarray, size_t ptrc, size_t size);
	PTRARRAY *ptrarray_put(PTRARRAY *ptrarray, void *ptr, size_t size);
	void ptrarray_free_ptrs(PTRARRAY *ptrarray);
	void ptrarray_free(PTRARRAY *ptrarray);
#endif

