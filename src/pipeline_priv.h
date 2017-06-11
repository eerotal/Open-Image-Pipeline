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

#ifndef PIPELINE_PRIV_INCLUDED
	#define PIPELINE_PRIV_INCLUDED

	#include "imgutil/imgutil.h"
	#include "job_priv.h"

	int pipeline_reg_progress_callback(void (*const callback)(const unsigned int progress));
	int pipeline_unreg_progress_callback(void (*const callback)(const unsigned int progress));

	int pipeline_feed(JOB *job);
	void pipeline_cleanup(void);
#endif
