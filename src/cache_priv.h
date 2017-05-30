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

#ifndef INCLUDED_CACHE_PRIV
	#define INCLUDED_CACHE_PRIV

	#include <stdlib.h>
	#include <time.h>

	typedef struct CACHE_FILE_STRUCT {
		char *fname;
		char *fpath;
		time_t tstamp;
	} CACHE_FILE;

	typedef struct CACHE_STRUCT {
		char *name;
		char *path;
		unsigned int max_files;

		CACHE_FILE **db;
		unsigned int db_len;
	} CACHE;

	CACHE *cache_create(const char *cache_name);
	void cache_destroy(CACHE *cache, int del_files);

	int cache_setup(void);
	void cache_cleanup(int del_files);

	int cache_db_unreg_file(CACHE *cache, const char *fname);
	CACHE_FILE *cache_db_reg_file(CACHE *cache, const char *fname);

	void cache_dump_all(void);
	void cache_dump(CACHE *cache);
	int cache_file_delete(CACHE *cache, const char *fname);
	int cache_has_file(CACHE *cache, const char *fname);
	CACHE *cache_get_cache_by_name(const char *name);
	char *cache_get_path_to_file(CACHE *cache, const char *fname);
#endif
