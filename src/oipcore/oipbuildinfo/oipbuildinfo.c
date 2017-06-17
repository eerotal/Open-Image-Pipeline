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

#define PRINT_IDENTIFIER "oipbuildinfo"

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>

#include "oipcore/abi/output.h"
#include "oipbuildinfo/oipbuildinfo.h"

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

#define VER_SUFFIX_DEBUG " (DEBUG)"

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
	char *str = NULL;
	str = build_get_version_string(prefix, info);
	if (str) {
		printf("%s\n", str);
		free(str);
	} else {
		printf("(unknown)\n");
	}
}

char *build_get_version_string(const char *prefix,
				const struct BUILD_INFO_STRUCT *info) {
	/*
	*  Return a version string constructed from data in info.
	*  Prefix is appended in front of the string. Returns a pointer
	*  to the new string on success or a NULL pointer on failure.
	*/
	char *ret = NULL;
	size_t ret_len = strlen(prefix) + strlen(info->version) + strlen(info->date);

	// Add space for abi->info after conversion to string.
	if (info->abi == 0) {
		ret_len += 1;
	} else {
		ret_len += (size_t) floor(log10(info->abi)) + 1;
	}

	// Add space for the VERSTR_DEBUG string.
	if (info->debug) {
		ret_len += strlen(VER_SUFFIX_DEBUG);
	}

	// Add space for the extra chars and the NULL byte.
	ret_len += 11 + 1;

	errno = 0;
	ret = calloc(ret_len, sizeof(char));
	if (!ret) {
		printerrno("calloc()");
		return NULL;
	}

	if (info->debug) {
		sprintf(ret, "%s%s - %s (ABI: %i)%s", prefix, info->version,
				info->date, info->abi, VER_SUFFIX_DEBUG);
	} else {
		sprintf(ret, "%s%s - %s (ABI: %i)", prefix, info->version,
				info->date, info->abi);
	}
	return ret;
}

