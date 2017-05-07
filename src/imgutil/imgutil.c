#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include "imgutil/imgutil.h"

IMAGE *img_load(char *path) {
	FREE_IMAGE_FORMAT ftype = FIF_UNKNOWN;
	FIBITMAP *fimage = NULL;
	FIBITMAP *fimage_converted = NULL;
	IMAGE *ret = NULL;

	if (access(path, F_OK)){
		printf("img_load(): %s\n", strerror(errno));
		return NULL;
	}
	ftype = FreeImage_GetFileType(path, 0);
	if (ftype == FIF_UNKNOWN) {
		ftype = FreeImage_GetFIFFromFilename(path);
		if (ftype == FIF_UNKNOWN) {
			printf("img_load(): Unknown filetype.\n");
			return NULL;
		}
	}
	if (FreeImage_FIFSupportsReading(ftype)) {
		fimage = FreeImage_Load(ftype, path, 0);
		if (!fimage) {
			printf("img_load(): Failed to load image.\n");
			return NULL;
		}

		fimage_converted = FreeImage_ConvertTo32Bits(fimage);
		if (!fimage_converted) {
			printf("img_load(): Failed to convert image to 32-bits.\n");
			return NULL;
		}

		ret = img_alloc(FreeImage_GetWidth(fimage_converted),
				FreeImage_GetHeight(fimage_converted));
		if (!ret) {
			printf("img_load(): Failed to allocate memory for image.\n");
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
	printf("img_load(): Image plugin doesn't support reading.\n");
	return NULL;
}

IMAGE *img_alloc(uint32_t w, uint32_t h) {
	IMAGE *ret = malloc(sizeof(IMAGE));
	if (!ret) {
		return NULL;
	}
	ret->w = w;
	ret->h = h;

	if (ret->w != 0 && ret->h != 0) {
		ret->img = malloc(img_bytelen(ret));
		if (!ret->img) {
			free(ret);
			return NULL;
		}
	} else {
		ret->img = NULL;
	}
	return ret;
}

int img_realloc(IMAGE *img, uint32_t w, uint32_t h) {
	RGBQUAD *tmp = realloc(img->img, w*h*sizeof(RGBQUAD));
	if (!tmp) {
		return 1;
	}
	img->img = tmp;
	img->w = w;
	img->h = h;
	return 0;
}

int img_cpy(IMAGE *dest, const IMAGE *src) {
	if (dest->w == src->w && dest->h == src->h) {
		memcpy(dest, src, img_bytelen(dest));
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
