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

#define PRINT_IDENTIFIER "pipeline"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

#include "headers/output.h"
#include "pipeline_priv.h"
#include "plugin_priv.h"
#include "file.h"
#include "cache_priv.h"

static int pipeline_write_cache(const JOB *job, unsigned int p_index, const IMAGE *img);
static int pipeline_load_cache(const JOB *job, IMAGE **dst);

static clock_t cputime_last = 0.0f;

static float pipeline_cputime(void) {
	/*
	*  Return the elapsed CPU time in seconds
	*  since this function was last called.
	*/

	clock_t cputime_current = clock();
	float ret = 0.0f;

	ret = (float) (cputime_current - cputime_last)/CLOCKS_PER_SEC;
	cputime_last = cputime_current;
	return ret;
}

static int pipeline_write_cache(const JOB *job, unsigned int p_index, const IMAGE *img) {
	/*
	*  Write the supplied image into the cache file of the plugin at 'p_index'.
	*  Returns 0 on success and 1 on failure.
	*/
	PLUGIN *tmp_plugin = NULL;
	CACHE_FILE *tmp_cache_file = NULL;

	tmp_plugin = plugin_get(p_index);
	if (tmp_plugin == NULL) {
		return 1;
	}

	tmp_cache_file = cache_db_file_reg(tmp_plugin->p_cache, job->job_id, 1);
	if (tmp_cache_file == NULL) {
		printerr("Failed to register cache file.\n");
		return 1;
	}

	printverb_va("Cache image: %s\n", tmp_cache_file->fpath);
	if (img_save(img, tmp_cache_file->fpath) != 0) {
		if (cache_db_file_unreg(tmp_plugin->p_cache, job->job_id) != 0) {
			printerr("Failed to unregister cache file.\n");
		}
		return 1;
	}
	return 0;
}

static int pipeline_load_cache(const JOB *job, IMAGE **dst) {
	/*
	*  Load the last up-to-date cache file of 'job'. The resulting
	*  image is loaded into *dst. If no up-to-date cache file is found,
	*  the contents of *dst are not modified and 0 is returned.
	*  On failure -1 is returned.
	*/

	IMAGE *tmp = NULL;
	unsigned int maxindex = 0;
	unsigned int first = 0;
	char *cache_fpath = NULL;

	if (job->prev_plugin_count == 0) {
		return 0;
	}

	if (plugins_get_count() <= job->prev_plugin_count) {
		maxindex = plugins_get_count();
	} else {
		maxindex = job->prev_plugin_count;
	}

	first = maxindex;
	for (unsigned int i = 0; i < maxindex; i++) {
		if (plugin_get(i)->arg_rev != job->prev_plugin_arg_revs[i] ||
			plugin_get(i)->uid != job->prev_plugin_uids[i] ||
			!cache_has_file(plugin_get(i)->p_cache, job->job_id)) {
			printverb_va("First changed plugin is %u.\n", i);
			first = i;
		}
	}

	cache_fpath = cache_get_path_to_file(plugin_get(first - 1)->p_cache, job->job_id);
	if (cache_fpath == NULL) {
		printerr("Failed to get cache file path.\n");
		return -1;
	}

	printverb_va("Loading image from cache: %s\n", cache_fpath);
	tmp = img_load(cache_fpath);
	if (tmp == NULL) {
		printerr("Failed to load cache image.\n");
		return -1;
	}
	*dst = tmp;
	return first;
}

int pipeline_feed(JOB *job) {
	/*
	*  Feed a processing job to the processing pipeline.
	*  The result is put into job->result_img.
	*  This function returns 0 on success and 1 on failure.
	*/

	struct PLUGIN_INDATA in;
	int ret = 0;
	int first = 0;

	float t_delta = 0;
	unsigned int throughput = 0;

	if (plugins_get_count() != 0) {
		job->status = JOB_STATUS_FAIL;

		in.src = job->src_img;
		in.dst = img_alloc(0, 0);
		if (!in.dst) {
			return 1;
		}

		first = pipeline_load_cache(job, &in.src);
		if (first < 0) {
			printerr("Cache loading failed.\n");
			first = 0;
		}

		for (size_t i = first; i < plugins_get_count(); i++) {
			pipeline_cputime();
			printinfo_va("Feeding image data to plugin %i.\n", i);

			in.args = plugin_get(i)->args;
			in.argc = plugin_get(i)->argc;

			// Feed the image data to individual plugins.
			if (plugin_feed(i, &in) != PLUGIN_STATUS_DONE) {
				printerr_va("Failed to use plugin %i.\n", i);
				continue;
			}

			// Calculate elapsed time and throughput.
			t_delta = pipeline_cputime();
			throughput = round(img_bytelen(in.src)/t_delta);
			printinfo_va("Took %f CPU seconds. Throughput %u B/s.\n", t_delta, throughput);

			// Save a copy of the result into the cache file.
			if (pipeline_write_cache(job, i, in.dst) != 0) {
				printerr("Failed to write cache file.\n");
			}

			// Free in.src if it doesn't point to the original image.
			if (in.src != job->src_img) {
				img_free(in.src);
			}

			in.src = in.dst;
			in.dst = img_alloc(0, 0);
			if (!in.dst) {
				img_free(in.src);
				return 1;
			}
		}

		if (img_realloc(job->result_img, in.src->w, in.src->h) == 0) {
			img_cpy(job->result_img, in.src);
			job->status = JOB_STATUS_SUCCESS;
		} else {
			ret = 1;
		}

		// Cleanup
		if (in.src != job->src_img) {
			img_free(in.src);
		}
		img_free(in.dst);

		// Update the plugin argument revisions and UIDs on success.
		if (job_store_plugin_config(job) != 0) {
			printerr("Failed to store plugin config in the job.\n");
			ret = 1;
		}
		return ret;
	}
	return 1;
}
