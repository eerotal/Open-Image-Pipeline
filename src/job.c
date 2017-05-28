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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <math.h>

#include "job_priv.h"
#include "headers/plugin.h"
#include "plugin_priv.h"

static long long last_job_id = 0;

JOB *job_create(const char *fpath) {
	/*
	*  Initialize a new job. This function returns
	*  a pointer to the newly allocated job or a NULL
	*  pointer on failure.
	*/
	JOB *job = NULL;
	errno = 0;
	job = malloc(sizeof(JOB));
	if (job == NULL) {
		perror("malloc(): ");
		return NULL;
	}
	memset(job, 0, sizeof(JOB));

	// Load source image.
	job->src_img = img_load(fpath);
	if (job->src_img == NULL) {
		return NULL;
	}

	// Preallocate the result image.
	job->result_img = img_alloc(0, 0);
	if (job->result_img == NULL) {
		return NULL;
	}

	// Copy the filepath to the job.
	errno = 0;
	job->filepath = calloc(strlen(fpath), sizeof(char));
	if (job->filepath == NULL) {
		perror("calloc(): ");
		return NULL;
	}
	strcpy(job->filepath, fpath);

	// Assign the supplied job a unique job ID.
	errno = 0;
	job->job_id = calloc(round(log10(last_job_id)) + 2, sizeof(char));
	if (job->job_id == NULL) {
		perror("calloc(): ");
		return NULL;
	}
	sprintf(job->job_id, "%llu", last_job_id);
	last_job_id++;

	job->status = JOB_STATUS_PENDING;

	return job;
}

int job_save_result(JOB *job, char *fpath) {
	return img_save(job->result_img, fpath);
}

int job_store_plugin_config(JOB *job) {
	unsigned long long int *tmp_arg_revs = NULL;
	unsigned long long int *tmp_uids = NULL;

	errno = 0;
	tmp_arg_revs = realloc(job->prev_plugin_arg_revs,
			plugins_get_count()*sizeof(unsigned long long int));
	if (tmp_arg_revs == NULL) {
		perror("realloc(): ");
		return 1;
	}
	job->prev_plugin_arg_revs = tmp_arg_revs;

	errno = 0;
	tmp_uids = realloc(job->prev_plugin_uids,
			plugins_get_count()*sizeof(unsigned long long int));
	if (tmp_uids == NULL) {
		perror("realloc(): ");
		return 1;
	}
	job->prev_plugin_uids = tmp_uids;

	for (unsigned int i = 0; i < plugins_get_count(); i++) {
		job->prev_plugin_arg_revs[i] = plugin_get(i)->arg_rev;
		job->prev_plugin_uids[i] = plugin_get(i)->uid;
	}
	job->prev_plugin_count = plugins_get_count();
	return 0;
}

void job_print(JOB *job) {
	printf("\n==== JOB ====\n");
	printf("    Filepath:        %s\n", job->filepath);
	printf("    ID:              %s\n", job->job_id);
	printf("    Status:          ");
	if (job->status == JOB_STATUS_FAIL) {
		printf("FAIL\n");
	} else if (job->status == JOB_STATUS_SUCCESS) {
		printf("SUCCESS\n");
	} else if (job->status == JOB_STATUS_PENDING) {
		printf("PENDING\n");
	}
	printf("    Plugin count:    %u\n", job->prev_plugin_count);
	printf("    Plugin arg revs: ");
	for (unsigned int i = 0; i < job->prev_plugin_count; i++) {
		printf("%llu ", job->prev_plugin_arg_revs[i]);
	}
	printf("\n");
	printf("    Plugin UIDs:     ");
	for (unsigned int i = 0; i < job->prev_plugin_count; i++) {
		printf("%llu ", job->prev_plugin_uids[i]);
	}
	printf("\n==== JOB ====\n\n");
}

void job_destroy(JOB *job) {
	/*
	*  Destroy a job and free all the resources
	*  allocated to it.
	*/
	if (job != NULL) {
		if (job->src_img != NULL) {
			img_free(job->src_img);
			job->src_img = NULL;
		}
		if (job->result_img != NULL) {
			img_free(job->result_img);
			job->result_img = NULL;
		}
		if (job->filepath != NULL) {
			free(job->filepath);
			job->filepath = NULL;
		}
		if (job->job_id != NULL) {
			free(job->job_id);
			job->job_id = NULL;
		}
		if (job->prev_plugin_arg_revs != NULL) {
			free(job->prev_plugin_arg_revs);
			job->prev_plugin_arg_revs = NULL;
		}
		if (job->prev_plugin_uids != NULL) {
			free(job->prev_plugin_uids);
			job->prev_plugin_uids = NULL;
		}
		free(job);
	}
}
