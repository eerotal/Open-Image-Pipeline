#ifndef INCLUDED_CACHE_PRIV
	#define INCLUDED_CACHE_PRIV

	char *cache_create(const char *name, unsigned int id);
	int cache_delete_all(void);
#endif