#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <math.h>

#include "job_priv.h"

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
		return NULL;
	}
	memset(job, 0, sizeof(JOB));

	// Load source image.
	errno = 0;
	job->src_img = img_load(fpath);
	if (job->src_img == NULL) {
		return NULL;
	}

	// Preallocate the result image.
	errno = 0;
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
	job->cache_id = calloc(round(log10(last_job_id)) + 2, sizeof(char));
	if (job->cache_id == NULL) {
		perror("calloc(): ");
		return NULL;
	}
	sprintf(job->cache_id, "%llu", last_job_id);
	last_job_id++;

	return job;
}

int job_save_result(JOB *job, char *fpath) {
	return img_save(job->result_img, fpath);
}

void job_destroy(JOB *job) {
	/*
	*  Destroy a job and free all the resources
	*  allocated to it.
	*/
	if (job != NULL) {
		if (job->src_img != NULL) {
			img_free(job->src_img);
		}
		if (job->result_img != NULL) {
			img_free(job->result_img);
		}
		if (job->filepath != NULL) {
			free(job->filepath);
		}
		if (job->cache_id != NULL) {
			free(job->cache_id);
		}
		free(job);
	}
}
