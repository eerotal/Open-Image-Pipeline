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

#include <stdio.h>
#include <string.h>

#include "build_priv.h"

int build_compare(struct OIP_BUILD_INFO_STRUCT *info) {
	/*
	*  Compare the OIP_BUILD_INFO_STRUCT of this build to the specified
	*  OIP_BUILD_INFO_STRUCT instance. Returns 1 if the contents are
	*  equal and 0 otherwise.
	*/

	if (strcmp(info->version, OIP_BUILD_INFO.version) != 0) {
		return 0;
	}
	if (strcmp(info->date, OIP_BUILD_INFO.date) != 0) {
		return 0;
	}
	if (info->debug != OIP_BUILD_INFO.debug) {
		return 0;
	}
	return 1;
}

void build_print_version_info(void) {
	printf("Version: %s - %s", OIP_BUILD_INFO.version, OIP_BUILD_INFO.date);
	if (OIP_BUILD_INFO.debug) {
		printf(" (DEBUG BUILD)");
	}
	printf("\n");
}
