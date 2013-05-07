/* msr_rapl.h
 */
#include <stdint.h>

// TLCC2 architecture uses 062D processors; 
// those are the only ones we care about.
#define USE_062D 1
#define USE_062A 0	
#define USE_063E 0

// Watts and seconds are actual watts and actual seconds, not
// scaled values.  The bit vector is the 64-bit values that is
// read from/written to the msr.

struct rapl_limit{
	double 		watts;		// User-friendly interface.
	double	 	seconds;
	uint64_t 	bit_vector;	// User-unfriendly interface.
	uint64_t	error;		// Error types TBD
};


// We're going to overload this interface a bit...
//
// rapl_set_limit()
//
//	a) If a pointer is null, do nothing.
//
//	b) If the bit_vector is nonzero, translate the bit_vector to watts and seconds 
//	and write the bit vector to the msr.
//
//	c) If the bit_vector is zero, translate the watts and seconds to the appropriate
//	bit_vector and write the bit_vector to the msr.
//
// rapl_get_limit()
//
//	a) If a pointer is null, do nothing.
//
//	b) If the bit_vector is nonzero, translate the bit_vector to watts and seconds.
//
//	c) If the bit_vector is zero, read the msr value into the bit_vector and 
//	translate into watts and seconds.
//
int rapl_set_limit( int package, struct rapl_limit* limit1, struct rapl_limit* limit2, struct rapl_limit* dram );
int rapl_get_limit( int package, struct rapl_limit* limit1, struct rapl_limit* limit2, struct rapl_limit* dram );

struct delta{
	double		dram_watts;		// Average watts over the delta
	double		dram_joules;		// Average joules over the delta
	double		pkg_watts;
	double		pkg_joules;
	double		seconds;		// Length of time over the delta
};

// If delta is NULL just update internal bookkeeping.
//void take_delta( int package, struct delta* delta );




