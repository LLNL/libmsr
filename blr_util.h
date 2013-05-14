#include <sys/time.h>
#include <stdio.h>
#ifndef BLR_UTIL_H
#define BLR_UTIL_H

#ifdef __cplusplus 
extern "C" {
#endif
	
// Returns stop-start in seconds.
double ts_delta( struct timeval *start, struct timeval *stop );

// Simple interface to mkstemp.  Returns null on error.
FILE * safe_mkstemp( const char *filetag );

// Simple interface to getenv when expected value is an int.
int get_env_int( const char *name, int *val );

#ifdef __cplusplus 
}
#endif

#endif // BLR_UTIL_H

