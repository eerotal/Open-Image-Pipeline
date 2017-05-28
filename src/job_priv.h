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
