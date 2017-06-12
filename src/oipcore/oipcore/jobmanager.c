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

#define PRINT_IDENTIFIER "jobmanager"

#include <stdio.h>
#include <string.h>

#include "oipcore/abi/output.h"
#include "oipcore/ptrarray.h"
#include "oipcore/jobmanager.h"

PTRARRAY_TYPE_DEF(JOB);

static PTRARRAY_TYPE(JOB) *jobs = NULL;

static void jobmanager_job_free_wrapper(void *job);

static void jobmanager_job_free_wrapper(void *job) {
	job_destroy((JOB*) job);
}

int jobmanager_setup(void) {
	/*
	*  Setup the jobmanager. Returns 0 on success and
	*  1 on failure.
	*/
	printverb("Setup\n");
	jobs = (PTRARRAY_TYPE(JOB)*) ptrarray_create(&jobmanager_job_free_wrapper);
	if (!jobs) {
		printerr("Failed to create PTRARRAY.\n");
		return 1;
	}
	return 0;
}

size_t jobmanager_get_count(void) {
	return jobs->ptrc;
}

JOB *jobmanager_get_job_by_id(char *job_id) {
	/*
	*  Return a pointer to the JOB instance with the
	*  id 'job_id' or a NULL pointer if the JOB is
	*  not found.
	*/
	for (size_t i = 0; i < jobs->ptrc; i++) {
		if (strcmp(jobs->ptrs[i]->job_id, job_id) == 0) {
			return jobs->ptrs[i];
		}
	}
	return NULL;
}

void jobmanager_list(void) {
	/*
	*  List all the registered JOBs.
	*/
	for (size_t i = 0; i < jobs->ptrc; i++) {
		job_print(jobs->ptrs[i]);
	}
}

int jobmanager_reg_job(JOB *job) {
	/*
	*  Register a job with the jobmanager. Returns 0 on
	*  success and 1 on failure.
	*/
	if (!job) {
		printerr("Won't attempt to register a NULL job.\n");
		return 1;
	}
	printverb_va("Register job '%s' (%s).\n", job->job_id, job->filepath);
	if (!ptrarray_put_ptr((PTRARRAY_TYPE(void)*) jobs, job)) {
		printerr("Failed to add job.\n");
		return 1;
	}
	return 0;
}

int jobmanager_unreg_job(JOB *job, int destroy_job) {
	/*
	*  Unregister a JOB from the jobmanager. If 'destroy_job'
	*  is zero the JOB instance itself is not free'd. Otherwise
	*  the JOB instance is free'd too. Returns 0 on success
	*  and 1 on failure.
	*/
	PTRARRAY_TYPE(JOB) *tmp_jobs = NULL;
	printverb_va("Unregister job '%s' (%s).\n", job->job_id, job->filepath);
	tmp_jobs = (PTRARRAY_TYPE(JOB)*) ptrarray_pop_ptr((PTRARRAY_TYPE(void)*) jobs,
								job, destroy_job);
	if (!tmp_jobs) {
		printerr("Failed to pop pointer from PTRARRAY.\n");
		return 1;
	}
	jobs = tmp_jobs;
	return 0;
}

void jobmanager_cleanup(int destroy_jobs) {
	/*
	*  Free allocated resources. If free_jobs is 0,
	*  the actual registered JOB instances are not free'd.
	*  Otherwise the registered JOB instances are free'd too.
	*/
	printverb("Cleanup.\n");
	if (jobs) {
		if (destroy_jobs) {
			printverb("Destroying jobs.\n");
			ptrarray_free_ptrs((PTRARRAY_TYPE(void)*) jobs);
		}
		ptrarray_free((PTRARRAY_TYPE(void)*) jobs);
		jobs = NULL;
	}
}
