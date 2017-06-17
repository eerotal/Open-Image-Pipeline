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

#ifndef INCLUDED_BUILD_PRIV
	#define INCLUDED_BUILD_PRIV

	#define BUILD_MATCH           0
	#define BUILD_MISMATCH_ABI   -1
	#define BUILD_MISMATCH_DEBUG -2

	struct BUILD_INFO_STRUCT {
		const char *version;
		const char *date;
		const int debug;
		const int abi;
	};

	#ifdef OIP_BINARY
		extern const struct BUILD_INFO_STRUCT OIP_BUILD_INFO;
	#else
		extern const struct BUILD_INFO_STRUCT OIP_BUILT_AGAINST;
	#endif

	int build_compare_critical(const struct BUILD_INFO_STRUCT *info1,
				const struct BUILD_INFO_STRUCT *info2);
	void build_print_version_info(const char *prefix,
				const struct BUILD_INFO_STRUCT *info);
	char *build_get_version_string(const char *prefix,
				const struct BUILD_INFO_STRUCT *info);
#endif
