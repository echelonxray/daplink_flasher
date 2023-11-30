#ifndef _HEADERINC_MAIN_H
#define _HEADERINC_MAIN_H

#define _POSIX_C_SOURCE 200809L
#define _DEFAULT_SOURCE

#include <stdio.h>

#define STDIN  0
#define STDOUT 1
#define STDERR 2

#define VERBOSITY_QUIET   0
#define VERBOSITY_NORMAL  1
#define VERBOSITY_VERBOSE 2
#define VERBOSITY_DEBUG   3

#define HELP_REM "Use command flags \"--help\", \"-h\", or \"-?\" to show help and command usage information.\n"
#define HELP_MSG "TODO: Help Message\n"

#define PRINT_USAGE() { /* TODO */ }

typedef struct {
	const char* image;
	      char* image_buffer;
	      char* image_path;
	      char* image_format;
	      char* image_offset;
	const char* chip;
	const char* probe;
} RawParameters;

extern int verbosity;
extern RawParameters raw_params;

#endif
