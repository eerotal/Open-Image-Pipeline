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

#ifndef IMGUTIL_INCLUDED
	#define IMGUTIL_INCLUDED

	#include <FreeImage.h>

	typedef struct STRUCT_IMAGE {
		RGBQUAD *img;
		uint32_t w;
		uint32_t h;
	} IMAGE;

	void img_free(IMAGE *img);
	size_t img_bytelen(const IMAGE *img);
	int img_cpy(IMAGE *dest, const IMAGE *src);
	int img_realloc(IMAGE *img, uint32_t w, uint32_t h);
	IMAGE *img_alloc(uint32_t w, uint32_t h);
	IMAGE *img_load(const char *path);
	int img_save(const IMAGE *img, const char *filename);
#endif

