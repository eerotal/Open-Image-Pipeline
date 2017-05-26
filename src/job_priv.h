#ifndef INCLUDED_JOB_PRIV
	#define INCLUDED_JOB_PRIV

	#include "imgutil/imgutil.h"

	#define JOB_STATUS_PENDING 0
	#define JOB_STATUS_SUCCESS 1
	#define JOB_STATUS_FAIL 2

	typedef struct STRUCT_JOB {
		IMAGE *src_img;
		IMAGE *result_img;
		char *cache_id;
		char *filepath;
		int status;
	} JOB;

	JOB *job_create(const char *fpath);
	int job_save_result(JOB *job, char *fpath);
	void job_destroy(JOB *job);
#endif
