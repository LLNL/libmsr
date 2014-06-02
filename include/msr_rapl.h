/* msr_rapl.h
 */
#ifndef MSR_RAPL_H
#define MSR_RAPL_H
#include <stdint.h>
#include <sys/time.h>
// TLCC2 architecture uses 062D processors; 
// those are the only ones we care about.
#define USE_062D 1
#define USE_062A 0	
#define USE_063E 0

// Watts and seconds are actual watts and actual seconds, not
// scaled values.  The bit vector is the 64-bit values that is
// read from/written to the msr.
struct rapl_data{
	uint64_t old_pkg_bits;
	uint64_t pkg_bits;

	uint64_t old_dram_bits;
	uint64_t dram_bits;

	double old_pkg_joules;
	double pkg_joules;

	double old_dram_joules;
	double dram_joules;

	struct timeval old_now;
	struct timeval now;

	double elapsed;
	double pkg_delta_joules;
	double pkg_watts;
	double dram_delta_joules;
	double dram_watts;


	uint64_t flags;
};

enum rapl_data_flags{
	RDF_REENTRANT	= 0x01,
	RDF_INIT        = 0x02
};

struct rapl_limit{
	double 		watts;		// User-friendly interface.
	double	 	seconds;
	uint64_t 	bits;		// User-unfriendly interface.
};


// We're going to overload this interface a bit...
//
// set_rapl_limit()
//
//	a) If a pointer is null, do nothing.
//
//	b) If the bit_vector is nonzero, translate the bit_vector to watts and seconds 
//	and write the bit vector to the msr.
//
//	c) If the bit_vector is zero, translate the watts and seconds to the appropriate
//	bit_vector and write the bit_vector to the msr.
//
// get_rapl_limit()
//
//	a) If a pointer is null, do nothing.
//
//	b) If the bit_vector is nonzero, translate the bit_vector to watts and seconds.
//
//	c) If the bit_vector is zero, read the msr value into the bit_vector and 
//	translate into watts and seconds.
//
#ifdef __cplusplus 
extern "C" {
#endif
void set_rapl_limit( const int socket, struct rapl_limit* limit1, struct rapl_limit* limit2, struct rapl_limit* dram );
void get_rapl_limit( const int socket, struct rapl_limit* limit1, struct rapl_limit* limit2, struct rapl_limit* dram );
void dump_rapl_limit( struct rapl_limit *L );

void read_rapl_data( const int socket, struct rapl_data *r );
void dump_rapl_data( struct rapl_data *r );

void dump_rapl_terse();
void dump_rapl_terse_label();
#ifdef __cplusplus 
}
#endif

#endif /*MSR_RAPL_H*/
