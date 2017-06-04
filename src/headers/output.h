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

#ifndef INCLUDED_OUTPUT_MACROS
	#define INCLUDED_OUTPUT_MACROS

	#include <stdio.h>
	#include <errno.h>

	#ifndef PRINT_IDENTIFIER
		#define PRINT_IDENTIFIER "unknown"
	#endif

	unsigned int print_verbose;

	#ifdef OIP_BINARY
		// Define printing control macros for the main OIP binary.
		#define print_verbose_on() print_verbose = 1;
		#define print_verbose_off() print_verbose = 0;
	#endif

	// Print an error message.
	#define printerr_va(format, ...) fprintf(stderr, "%s: "format, PRINT_IDENTIFIER, __VA_ARGS__);
	#define printerr(format) fprintf(stderr, "%s: "format, PRINT_IDENTIFIER);

	// Print a text description of an error.
	#define printerrno(format) fprintf(stderr, "%s: "format": %s\n", PRINT_IDENTIFIER, strerror(errno));

	// Print an informational message.
	#define printinfo_va(format, ...) fprintf(stdout, "%s: "format, PRINT_IDENTIFIER, __VA_ARGS__);
	#define printinfo(format) fprintf(stdout, "%s: "format, PRINT_IDENTIFIER);

	// Print a verbose informational message.
	#define printverb_va(format, ...) {						\
		if (print_verbose) {							\
			fprintf(stdout, "%s: "format, PRINT_IDENTIFIER, __VA_ARGS__); 	\
		}									\
	} 										\

	#define printverb(format) {							\
		if (print_verbose) {							\
			fprintf(stdout, "%s: "format, PRINT_IDENTIFIER); 		\
		}									\
	} 										\

#endif
