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

#ifndef INCLUDED_CONFIGLOADER_PRIV
	#define INCLUDE_CONFIGLOADER_PRIV

	#include "oipdataparser/oipdataparser.h"

	PTRARRAY_TYPE(DP_VAR) *config_get(void);
	long config_get_lint_param(const char *param, size_t index);
	char *config_get_str_param(const char *param, size_t index);

	int config_load(char *cfpath);
	void config_cleanup(void);
#endif
