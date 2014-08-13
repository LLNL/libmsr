/* msr_clocks.h
 *
 * Copyright (c) 2011, 2012, 2013, 2014 by Lawrence Livermore National Security, LLC. LLNL-CODE-645430 
 * Written by Kathleen Shoga and Barry Rountree (shoga1|rountree@llnl.gov).
 * This file is part of libmsr and is licensed under the LGLP.  See the LICENSE file for details.
 */
#ifndef MSR_CLOCKS_H
#define MSR_CLOCKS_H

void read_all_aperf(uint64_t *aperf); 
void read_all_mperf(uint64_t *mperf); 
void read_all_tsc  (uint64_t *tsc); 
void dump_clocks_terse(FILE *w);
void dump_clocks_terse_label(FILE *w);

struct clock_mod{
	uint64_t raw;

// There is a bit at 0 that can be used for Extended On-Demand Clock Modulation Duty Cycle
// It is added with the bits 3:1. When used, the granularity of clock modulation duty cycle
// is increased to 6.25% as opposed to 12.5%
// To enable this, must have CPUID.06H:EAX[Bit 5] = 1
// I am not sure how to check that because otherwise bit 0 is reserved

	int duty_cycle;		// 3 binary digits
				// 0-7 in decimal
				//
				// Value	Duty Cycle
				//   0		 Reserved
				//   1		 12.5% (default)
				//   2		 25.0%
				//   3		 37.5%
				//   4		 50.0%
				//   5		 63.5%
				//   6		 75.0%
				//   7		 87.5%

	int duty_cycle_enable;	// Read/Write
};

void dump_clock_mod(struct clock_mod *s, FILE *w);
void get_clock_mod(int socket, int core, struct clock_mod *s);
void set_clock_mod(int socket, int core, struct clock_mod *s);

#endif //MSR_CLOCKS_H
