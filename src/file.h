#ifndef INCLUDED_FILE
	#define INCLUDED_FILE

	char *path_ensure_trailing_slash(const char *path);
	char *file_path_join(const char *s1, const char *s2);
	int rmdir_recursive(const char *rpath);
#endif
