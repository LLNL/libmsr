/*
 * Stub for systems that don't have libmsr
 */
#ifndef MSR_STUB_H
#define MSR_STUB_H

#include <stdint.h>

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

struct rapl_limit{
    double      watts;      // User-friendly interface.
    double      seconds;
    uint64_t    bits;       // User-unfriendly interface.
};

static inline void set_rapl_limit( const int socket, struct rapl_limit* limit1, struct rapl_limit* limit2, struct rapl_limit* dram ) {}
static inline void get_rapl_limit( const int socket, struct rapl_limit* limit1, struct rapl_limit* limit2, struct rapl_limit* dram ) {}

static inline void read_rapl_data( const int socket, struct rapl_data *r ) {}

#endif
