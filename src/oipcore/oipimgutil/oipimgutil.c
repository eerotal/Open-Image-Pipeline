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

#define PRINT_IDENTIFIER "imgutil"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>

#include "oipimgutil/oipimgutil.h"
#include "oipcore/abi/output.h"

#define IMGUTIL_OUTPUT_FORMAT FIF_JPEG
#define IMGUTIL_INTERNAL_BPP 32

IMAGE *img_load(const char *path) {
	FREE_IMAGE_FORMAT ftype = FIF_UNKNOWN;
	FIBITMAP *fimage = NULL;
	FIBITMAP *fimage_converted = NULL;
	IMAGE *ret = NULL;

	errno = 0;
	if (access(path, F_OK)){
		printerrno("access()");
		return NULL;
	}
	ftype = FreeImage_GetFileType(path, 0);
	if (ftype == FIF_UNKNOWN) {
		ftype = FreeImage_GetFIFFromFilename(path);
		if (ftype == FIF_UNKNOWN) {
			printerr("img_load(): Unknown filetype.\n");
			return NULL;
		}
	}
	if (FreeImage_FIFSupportsReading(ftype)) {
		fimage = FreeImage_Load(ftype, path, 0);
		if (!fimage) {
			printerr("img_load(): Failed to load image.\n");
			return NULL;
		}

		fimage_converted = FreeImage_ConvertTo32Bits(fimage);
		if (!fimage_converted) {
			printerr("img_load(): Failed to convert image to 32-bits.\n");
			return NULL;
		}

		ret = img_alloc(FreeImage_GetWidth(fimage_converted),
				FreeImage_GetHeight(fimage_converted));
		if (!ret) {
			printerr("img_load(): Failed to allocate memory for image.\n");
			FreeImage_Unload(fimage);
			FreeImage_Unload(fimage_converted);
			return NULL;
		}
		memcpy(ret->img, (RGBQUAD*) FreeImage_GetBits(fimage_converted),
			img_bytelen(ret));

		FreeImage_Unload(fimage);
		FreeImage_Unload(fimage_converted);
		return ret;
	}
	printerr("img_load(): Image plugin doesn't support reading.\n");
	return NULL;
}

int img_save(const IMAGE *img, const char *filename) {
	FIBITMAP *bitmap = FreeImage_Allocate(img->w, img->h, IMGUTIL_INTERNAL_BPP, 0, 0, 0);
	if (!bitmap) {
		return 1;
	}
	memcpy(FreeImage_GetBits(bitmap), img->img, img_bytelen(img));
	if (!FreeImage_Save(IMGUTIL_OUTPUT_FORMAT, FreeImage_ConvertTo24Bits(bitmap), filename, 0)) {
		FreeImage_Unload(bitmap);
		return 1;
	}
	FreeImage_Unload(bitmap);
	return 0;
}

IMAGE *img_alloc(uint32_t w, uint32_t h) {
	IMAGE *ret = NULL;
	errno = 0;
	ret = malloc(sizeof(IMAGE));
	if (!ret) {
		printerrno("malloc()");
		return NULL;
	}
	ret->w = w;
	ret->h = h;

	if (ret->w != 0 && ret->h != 0) {
		errno = 0;
		ret->img = malloc(img_bytelen(ret));
		if (!ret->img) {
			printerrno("malloc()");
			free(ret);
			return NULL;
		}
	} else {
		ret->img = NULL;
	}
	return ret;
}

int img_realloc(IMAGE *img, uint32_t w, uint32_t h) {
	RGBQUAD *tmp = NULL;
	errno = 0;
	tmp = realloc(img->img, w*h*sizeof(RGBQUAD));
	if (!tmp) {
		printerrno("realloc()");
		return 1;
	}
	img->img = tmp;
	img->w = w;
	img->h = h;
	return 0;
}

int img_cpy(IMAGE *dest, const IMAGE *src) {
	if (dest->w == src->w && dest->h == src->h) {
		memcpy(dest->img, src->img, img_bytelen(dest));
		return 0;
	} else {
		return 1;
	}
}

size_t img_bytelen(const IMAGE *img) {
	return img->w*img->h*sizeof(RGBQUAD);
}

void img_free(IMAGE *img) {
	free(img->img);
	free(img);
}
