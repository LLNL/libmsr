/* msr_clocks.h
 *
 * Copyright (c) 2011-2015, Lawrence Livermore National Security, LLC. LLNL-CODE-645430
 * Produced at Lawrence Livermore National Laboratory  
 * Written by  Barry Rountree, rountree@llnl.gov
 *             Scott Walker,   walker91@llnl.gov
 *             Kathleen Shoga, shoga1@llnl.gov
 *
 * All rights reserved. 
 * 
 * This file is part of libmsr.
 * 
 * libmsr is free software: you can redistribute it and/or modify it under the
 * terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option) any
 * later version.
 * 
 * libmsr is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
 * PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 * details.
 * 
 * You should have received a copy of the GNU Lesser General Public License along
 * with libmsr.  If not, see <http://www.gnu.org/licenses/>. 
 *
 * This material is based upon work supported by the U.S. Department
 * of Energy's Lawrence Livermore National Laboratory. Office of
 * Science, under Award number DE-AC52-07NA27344.
 *
 */

#ifndef MSR_CLOCKS_H
#define MSR_CLOCKS_H
#include <stdint.h>

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

#ifdef __cplusplus
extern "C" {
#endif

void read_all_clocks(uint64_t ** aperf, uint64_t ** mperf, uint64_t ** tsc);
void read_all_aperf(uint64_t **aperf);
void read_all_mperf(uint64_t **mperf);
void read_all_tsc  (uint64_t **tsc);
void dump_clocks_terse(FILE *w);
void dump_clocks_terse_label(FILE *w);

void dump_clocks_readable(FILE * writeFile);
void dump_clock_mod(struct clock_mod *s, FILE *w);
void get_clock_mod(int socket, int core, struct clock_mod *s);
void set_clock_mod(int socket, int core, struct clock_mod *s);

#ifdef __cplusplus
}
#endif

#endif //MSR_CLOCKS_H
