#ifndef INCLUDED_CACHE_PRIV
	#define INCLUDED_CACHE_PRIV

	char *cache_get_root(void);
	char *cache_get_path(const char *cache_name);
	char *cache_create(const char *cache_name);
	int cache_file_exists(char *cache_name, char *cache_id);
	int cache_delete_all(void);
#endif
