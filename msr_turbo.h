#ifndef MSR_TURBO_H
#define MSR_TURBO_H

#ifdef __cplusplus 
extern "C" {
#endif

// NOTE:  this interfaces assumes turbo is present on the
// processor and has been turned on in the BIOS.  See section
// 14.3.2.2 of volume 3B of Intel's documentation (March 2013
// edition) for details.  

// Also note that the IA32_PERF_CTL (0x199) is a per-thread
// msr, but that turbo mode is a per-processor decision.  This
// interface is implemented at a per-core level (for now) and
// disabling turbo for all cores appears to be sufficient for
// disabling turbo on the processor.  The effects of disabling
// turbo on a subset of cores (or threads) is unknown (at least
// by us).


void enable_turbo( const int socket );
void disable_turbo( const int socket );
void enable_all_turbo( );
void disable_all_turbo( );
void dump_turbo_state( );

#ifdef __cplusplus 
}
#endif

#endif //MSR_TURBO_H


