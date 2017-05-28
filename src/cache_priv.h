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

	char *cache_get_root(void);
	char *cache_get_path(const char *cache_name);
	char *cache_get_file_path(const char *cache_name, const char *cache_id);
	char *cache_create(const char *cache_name);
	int cache_file_exists(const char *cache_name, const char *cache_id);
	int cache_delete_all(void);
#endif
