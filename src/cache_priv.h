#ifndef INCLUDED_CACHE_PRIV
	#define INCLUDED_CACHE_PRIV

	char *cache_get_root(void);
	char *cache_get_path(const char *cache_name);
	char *cache_get_file_path(const char *cache_name, const char *cache_id);
	char *cache_create(const char *cache_name);
	int cache_file_exists(const char *cache_name, const char *cache_id);
	int cache_delete_all(void);
#endif
