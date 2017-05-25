#ifndef INCLUDED_JOB_PRIV
	#define INCLUDED_JOB_PRIV

	#include "imgutil/imgutil.h"

	typedef struct STRUCT_JOB {
		IMAGE *src_img;
		IMAGE *result_img;
		char *cache_id;
		char *filepath;
	} JOB;

	JOB *job_create(const char *fpath);
	int job_save_result(JOB *job, char *fpath);
	void job_destroy(JOB *job);
#endif
