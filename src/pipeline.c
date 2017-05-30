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
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

#include "pipeline_priv.h"
#include "plugin_priv.h"
#include "file.h"
#include "cache_priv.h"

static int pipeline_write_cache(const IMAGE *img, unsigned int p_index, char *uuid);
static int pipeline_get_first_changed_plugin(const JOB *job);

static int pipeline_write_cache(const IMAGE *img, unsigned int p_index, char *cache_id) {
	/*
	*  Write the supplied image into the cache of the supplied plugin.
	*  This function returns 0 on success and 1 on failure.
	*/
	PLUGIN *tmp_plugin = NULL;
	char *c_fullpath = NULL;

	tmp_plugin = plugin_get(p_index);
	if (tmp_plugin == NULL) {
		return 1;
	}

	c_fullpath = cache_get_path_to_file(tmp_plugin->p_cache, cache_id);
	if (c_fullpath == NULL) {
		return 1;
	}

	printf("pipeline: Cache image: %s\n", c_fullpath);
	if (img_save(img, c_fullpath) != 0) {
		free(c_fullpath);
		return 1;
	}

	// Register the new cache file to the caching system.
	cache_db_reg_file(tmp_plugin->p_cache, cache_id);

	free(c_fullpath);
	return 0;
}

static int pipeline_get_first_changed_plugin(const JOB *job) {
	/*
	*  Return the ID of the first plugin that matches one or more
	*  of the following conditions.
	*  1. No cache file exists.
	*  2. Plugin UID at a specific index has changed since last feed.
	*  3. Argument revision has changed since last feed.
	*  Additionally, if job->prev_plugin_count == 0, 0 is returned.
	*  If no plugin fulfilling these conditions is found, return
	*  the total number of plugins in the pipeline.
	*/

	unsigned int maxindex = 0;
	if (job->prev_plugin_count == 0) {
		return 0;
	}

	if (plugins_get_count() <= job->prev_plugin_count) {
		maxindex = plugins_get_count();
	} else {
		maxindex = job->prev_plugin_count;
	}

	for (unsigned int i = 0; i < maxindex; i++) {
		if (plugin_get(i)->arg_rev != job->prev_plugin_arg_revs[i] ||
			plugin_get(i)->uid != job->prev_plugin_uids[i] ||
			!cache_has_file(plugin_get(i)->p_cache, job->job_id)) {
			printf("pipeline: First changed plugin is %u.\n", i);
			return i;
		}
	}
	return plugins_get_count();
}

int pipeline_feed(JOB *job) {
	/*
	*  Feed a processing job to the processing pipeline.
	*  The result is put into job->result_img.
	*  This function returns 0 on success and 1 on failure.
	*/

	char *cache_file_path = NULL;
	int ret = 0;
	clock_t t_start = 0;
	float delta_t = 0;
	IMAGE *buf_ptr_1 = NULL;
	IMAGE *buf_ptr_2 = NULL;

	if (plugins_get_count() != 0) {
		// Set the job status to fail initially and correct it later.
		job->status = JOB_STATUS_FAIL;

		buf_ptr_1 = job->src_img;
		buf_ptr_2 = img_alloc(0, 0);
		if (!buf_ptr_2) {
			return 1;
		}

		// Get the first plugin whose output should be generated.
		unsigned int i = pipeline_get_first_changed_plugin(job);

		if (i != 0) {
			/*
			*  Skip plugins by starting the pipeline at the index i.
			*  Load the input data from the cache file of the last
			*  plugin that hasn't changed.
			*/

			cache_file_path = cache_get_path_to_file(plugin_get(i - 1)->p_cache, job->job_id);
			if (cache_file_path == NULL) {
				printf("pipeline: Failed to get cache file path.\n");
				img_free(buf_ptr_2);
				return 1;
			}

			printf("pipeline: Loading image from cache: %s\n", cache_file_path);
			buf_ptr_1 = img_load(cache_file_path);
			if (buf_ptr_1 == NULL) {
				img_free(buf_ptr_2);
				return 1;
			}
		}

		for (; i < plugins_get_count(); i++) {
			printf("pipeline: Feeding image data to plugin %i.\n", i);
			t_start = clock();

			// Feed the image data to individual plugins.
			if (plugin_feed(i, (const char**)plugin_get(i)->args, plugin_get(i)->argc,
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
			if (pipeline_write_cache(buf_ptr_2, i, job->job_id) != 0) {
				printf("pipeline: Failed to write cache file.\n");
			}

			/*
			*  Only free buf_ptr_1 if it doesn't point to
			*  the original image, which is declared const.
			*/
			if (buf_ptr_1 != job->src_img) {
				img_free(buf_ptr_1);
			}

			buf_ptr_1 = buf_ptr_2;
			buf_ptr_2 = img_alloc(0, 0);
			if (!buf_ptr_2) {
				img_free(buf_ptr_1);
				return 1;
			}
		}

		if (img_realloc(job->result_img, buf_ptr_1->w, buf_ptr_1->h) == 0) {
			memcpy(job->result_img->img, buf_ptr_1->img, img_bytelen(job->result_img));

			// Set the job status to success.
			job->status = JOB_STATUS_SUCCESS;
		} else {
			ret = 1;
		}

		// Cleanup
		if (buf_ptr_1 != job->src_img) {
			img_free(buf_ptr_1);
		}
		img_free(buf_ptr_2);

		// Update the plugin argument revisions and UIDs on a success.
		if (job_store_plugin_config(job) != 0) {
			printf("pipeline: Failed to store plugin config in the job.\n");
			return 1;
		}
		return ret;
	} else {
		return 1;
	}
}
