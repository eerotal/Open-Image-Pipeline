#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

#include "pipeline_priv.h"
#include "plugin_priv.h"
#include "file.h"
#include "cache_priv.h"

static int pipeline_write_cache(const IMAGE *img, unsigned int p_index, char *uuid);

static int pipeline_write_cache(const IMAGE *img, unsigned int p_index, char *cache_id) {
	/*
	*  Write the supplied image into the cache of the supplied plugin.
	*  This function returns 0 on success and 1 on failure.
	*/
	char *c_dir = NULL;
	char *c_fullpath = NULL;

	c_dir = plugin_get_cache_path(p_index);
	if (c_dir == NULL) {
		return 1;
	}

	c_fullpath = file_path_join(c_dir, cache_id);
	if (c_fullpath == NULL) {
		return 1;
	}

	printf("pipeline: Cache image: %s\n", c_fullpath);
	img_save(img, c_fullpath);
	free(c_fullpath);

	return 0;
}

int pipeline_feed(const IMAGE *img, IMAGE *result, char *cache_id) {
	/*
	*  Feed an image to the processing pipeline.
	*  The result is put into 'result', which needs
	*  to be allocated but it's size doesn't matter.
	*  This function will reallocate the internal pixel
	*  buffer anyway. This function returns 0 on success
	*  and 1 on failure.
	*/

	int ret = 0;
	clock_t t_start = 0;
	float delta_t = 0;
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

			// Check if a cached output file for this plugin already exists.
			if (cache_file_exists(plugin_get_cache_name(i), cache_id)) {
				printf("pipeline: Cache file exists.\n");
			}

			// Feed the image data to individual plugins.
			if (plugin_feed(i, (const char**)plugin_args_get(i), plugin_args_get_count(i),
					buf_ptr_1, buf_ptr_2) != 0) {

				printf("pipeline: Failed to use plugin %i.\n", i);
				continue;
			}

			// Calculate elapsed time and avg throughput.
			delta_t = (float) (clock() - t_start)/CLOCKS_PER_SEC;
			printf("pipeline: Data processed in %f CPU seconds.\n", delta_t);
			printf("pipeline: Avg throughput: %i B/s.\n",
				(int) round(img_bytelen(buf_ptr_1)/delta_t));

			// Save a copy of the result into the cache file.
			if (pipeline_write_cache(buf_ptr_2, i, cache_id) != 0) {
				printf("pipeline: Failed to write cache file.\n");
			}

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
