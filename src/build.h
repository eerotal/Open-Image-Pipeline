#ifndef INCLUDED_VERSION
	#define INCLUDED_VERSION

	/*
	*  Constants defined at build time.
	*/
	#define STRINGIFY(x) #x
	#define STRINGIFY_EXP(x) STRINGIFY(x)

	const char *build_version = STRINGIFY_EXP(BUILD_VERSION);
	const char *build_date = STRINGIFY_EXP(BUILD_DATE);
	const int build_debug = BUILD_DEBUG;
#endif
