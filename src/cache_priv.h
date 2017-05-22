#ifndef INCLUDED_CACHE_PRIV
	#define INCLUDED_CACHE_PRIV

	char *cache_get_dir(void);
	char *cache_create(const char *name);
	int cache_delete_all(void);
#endif
