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

#ifndef INCLUDED_DATAPARSER
	#define INCLUDED_DATAPARSER

	#include "oipcore/ptrarray.h"

	typedef struct {
		char *var;
		PTRARRAY_TYPE(char) *data;
	} DP_VAR;

	PTRARRAY_TYPE_DEF(DP_VAR);

	void dp_dump(DP_VAR *var);
	DP_VAR *dp_parse_single(const char *str);
	PTRARRAY_TYPE(DP_VAR) *dp_parse_multipart(const char *str);

	int dp_var_validate_numeric(DP_VAR *var);

	PTRARRAY_TYPE(char) *dp_var_strarr(DP_VAR *var);
	PTRARRAY_TYPE(long) *dp_var_lintarr(DP_VAR *var);
#endif
