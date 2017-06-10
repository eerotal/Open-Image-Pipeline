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

#include "buildinfo/build.h"

// Build info constants defined at build time.
#ifdef OIP_BINARY
	const struct BUILD_INFO_STRUCT OIP_BUILD_INFO = {
		.version = OIP_BUILD_VERSION,
		.date = OIP_BUILD_DATE,
		.debug = OIP_BUILD_DEBUG,
		.abi = OIP_BUILD_ABI
	};
#else
	const struct BUILD_INFO_STRUCT OIP_BUILT_AGAINST = {
		.version = OIP_BUILD_VERSION,
		.date = OIP_BUILD_DATE,
		.debug = OIP_BUILD_DEBUG,
		.abi = OIP_BUILD_ABI
	};
#endif

int build_compare_critical(const struct BUILD_INFO_STRUCT *info1,
				const struct BUILD_INFO_STRUCT *info2) {
	/*
	*  Compare the critical parts of build information in info1 and info2.
	*  The ABI version and debug build information are critical for example.
	*  This function is basically used to test whether plugins can be loaded
	*  into the main OIP binary without causing a crash. This function returns
	*  one of the BUILD_MISMATCH_* constants if there's a mismatch or
	*  BUILD_MATCH if the builds match.
	*/

	if (info1->abi != info2->abi) {
		return BUILD_MISMATCH_ABI;
	} else if (info1->debug != info2->debug) {
		return BUILD_MISMATCH_DEBUG;
	}
	return BUILD_MATCH;
}

void build_print_version_info(const char *prefix,
			const struct BUILD_INFO_STRUCT *info) {
	printf("%s%s - %s (ABI: %i)", prefix, info->version, info->date, info->abi);
	if (info->debug) {
		printf(" (DEBUG BUILD)");
	}
	printf("\n");
}
