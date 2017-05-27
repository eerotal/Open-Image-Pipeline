#ifndef INCLUDED_JOB_PRIV
	#define INCLUDED_JOB_PRIV

	#include "imgutil/imgutil.h"

	#define JOB_STATUS_PENDING 0
	#define JOB_STATUS_SUCCESS 1
	#define JOB_STATUS_FAIL 2

	typedef struct STRUCT_JOB {
		IMAGE *src_img;
		IMAGE *result_img;
		char *job_id;
		char *filepath;
		unsigned long long int *prev_plugin_uids;
		unsigned long long int *prev_plugin_arg_revs;
		unsigned int prev_plugin_count;
		int status;
	} JOB;

	JOB *job_create(const char *fpath);
	int job_save_result(JOB *job, char *fpath);
	int job_store_plugin_config(JOB *job);
	void job_print(JOB *job);
	void job_destroy(JOB *job);
#endif
