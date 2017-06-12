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

#ifndef INCLUDED_JOBMANAGER
	#define INCLUDED_JOBMANAGER

	#include "oipcore/job.h"

	int jobmanager_setup(void);
	void jobmanager_cleanup(int destroy_jobs);

	void jobmanager_list(void);
	size_t jobmanager_get_count(void);
	JOB *jobmanager_get_job_by_id(char *job_id);

	int jobmanager_reg_job(JOB *job);
	int jobmanager_unreg_job(JOB *job, int destroy_job);
#endif
