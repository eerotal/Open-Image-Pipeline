#ifndef PIPELINE_PRIV_INCLUDED
	#define PIPELINE_PRIV_INCLUDED

	#include "imgutil/imgutil.h"

	char *pipeline_gen_new_cache_id(void);
	int pipeline_feed(const IMAGE *img, IMAGE *result, char *data_id);
#endif
