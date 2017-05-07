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
	IMAGE *img_load(char *path);
	int img_save(const IMAGE *img, const char *filename);
#endif

