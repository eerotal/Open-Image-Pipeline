#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "pipeline_priv.h"
#include "plugin_priv.h"

int pipeline_feed(const IMAGE *img, IMAGE *result) {
	int ret = 0;
	clock_t t_start = 0;
	IMAGE *buf_ptr_1 = NULL;
	IMAGE *buf_ptr_2 = NULL;

	if (img == NULL || result == NULL) {
		return 1;
	}

	if (plugins_get_count() != 0) {
		buf_ptr_1 = (IMAGE*) img;
		buf_ptr_2 = img_alloc(0, 0);
		if (!buf_ptr_2) {
			return 1;
		}

		for (unsigned int i = 0; i < plugins_get_count(); i++) {
			printf("pipeline: Feeding image data to plugin %i.\n", i);
			t_start = clock();
			if (plugin_feed(i, (const char**)plugin_args_get(i), plugin_args_get_count(i),
					buf_ptr_1, buf_ptr_2) != 0) {

				printf("pipeline: Failed to use plugin %i.\n", i);
				continue;
			}
			printf("pipeline: Data processed in %f CPU seconds.\n",
				(float) (clock() - t_start)/CLOCKS_PER_SEC);

			/*
			*  Only free buf_ptr_1 if it doesn't point to
			*  the original image, which is declared const.
			*/
			if (buf_ptr_1 != img) {
				img_free(buf_ptr_1);
			}

			buf_ptr_1 = buf_ptr_2;
			buf_ptr_2 = img_alloc(0, 0);
			if (!buf_ptr_2) {
				img_free(buf_ptr_1);
				return 1;
			}
		}

		if (img_realloc(result, buf_ptr_1->w, buf_ptr_1->h) == 0) {
			memcpy(result->img, buf_ptr_1->img, img_bytelen(result));
		} else {
			ret = 1;
		}

		// Cleanup
		if (buf_ptr_1 != img) {
			img_free(buf_ptr_1);
		}
		img_free(buf_ptr_2);
		return ret;
	} else {
		return 1;
	}
}
