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

#ifndef INCLUDED_BUILD
	#define INCLUDED_BUILD

	/*
	*  Version and build information constants defined
	*  by the build-config.sh script.
	*/

	#include "build_priv.h"

	const struct OIP_BUILD_INFO_STRUCT OIP_BUILD_INFO = {
		.version = "9dd83c0-dirty",
		.date = "Thu Jun  8 22:25:19 EEST 2017",
		.debug = 0
	};

#endif
