/* msr_clocks.c
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

#include <stdio.h>
#include "msr_core.h"
#include "msr_clocks.h"
#include "memhdlr.h"


#define MSR_IA32_MPERF 		0x000000e7
#define MSR_IA32_APERF 		0x000000e8
#define IA32_TIME_STAMP_COUNTER 0x00000010
#define IA32_CLOCK_MODULATION		(0x19A) // If Hyper-Threading Technology enabled processors, 
						// The IA32_CLOCK_MODULATION register is duplicated
						// for each logical processor. 
						// Must have it enabled and the same for all logical
						// processors within the physical processor

int clocks_storage(uint64_t *** aperf_val, uint64_t *** mperf_val, uint64_t *** tsc_val)
{
    static int init = 1;
    static uint64_t ** aperf = NULL, ** mperf = NULL, ** tsc = NULL;
    static uint64_t totalThreads = 0;
    if (init)
    {
        totalThreads = num_devs();
        aperf = (uint64_t **) libmsr_malloc(totalThreads * sizeof(uint64_t *));
        mperf = (uint64_t **) libmsr_malloc(totalThreads * sizeof(uint64_t *));
        tsc = (uint64_t **) libmsr_malloc(totalThreads * sizeof(uint64_t *));
        allocate_batch(CLOCKS_DATA, 3UL * num_devs());
        load_thread_batch(MSR_IA32_APERF, aperf, CLOCKS_DATA);
        load_thread_batch(MSR_IA32_MPERF, mperf, CLOCKS_DATA);
        load_thread_batch(IA32_TIME_STAMP_COUNTER, tsc, CLOCKS_DATA);
        init = 0;
    }
    if (aperf_val)
    {
        *aperf_val = aperf;
    }
    if (mperf_val)
    {
        *mperf_val = mperf;
    }
    if (tsc_val)
    {
        *tsc_val = tsc;
    }
    return 0;
}

void
dump_clocks_terse_label(FILE *writeFile){
	int thread_idx;
    static uint64_t totalThreads = 0;
    if (!totalThreads)
    {
        totalThreads = num_devs();
    }
	for(thread_idx=0; thread_idx<totalThreads; thread_idx++){
		fprintf(writeFile, "aperf%02d mperf%02d tsc%02d ", 
			thread_idx, thread_idx, thread_idx);
	}
}

void
dump_clocks_terse(FILE *writeFile){
    static uint64_t totalThreads = 0;
    static uint64_t ** aperf_val = NULL, 
                    ** mperf_val = NULL, 
                    ** tsc_val = NULL;
    if (!totalThreads)
    {
        totalThreads = num_devs();
        clocks_storage(&aperf_val, &mperf_val, &tsc_val);
    }
	int thread_idx;
    read_batch(CLOCKS_DATA);
	for(thread_idx=0; thread_idx<totalThreads; thread_idx++){
		fprintf(writeFile, "%20lu %20lu %20lu ", 
			*aperf_val[thread_idx], *mperf_val[thread_idx], *tsc_val[thread_idx]);
	}
}

void dump_clocks_readable(FILE * writeFile)
{
    static uint64_t totalThreads = 0;
    static uint64_t ** aperf_val = NULL, 
                    ** mperf_val = NULL, 
                    ** tsc_val = NULL;
    if (!totalThreads)
    {
        totalThreads = num_devs();
        clocks_storage(&aperf_val, &mperf_val, &tsc_val);
    }
#ifdef LIBMSR_DEBUG
    fprintf(stderr, "DEBUG: (clocks_readable) totalThreads is %lu\n", totalThreads);
#endif
	int thread_idx;
    read_batch(CLOCKS_DATA);
	for(thread_idx=0; thread_idx<totalThreads; thread_idx++){
		fprintf(writeFile, "aperf%02d:%20lu mperf%02d:%20lu tsc%02d:%20lu\n", 
			thread_idx, *aperf_val[thread_idx], thread_idx, *mperf_val[thread_idx], 
            thread_idx, *tsc_val[thread_idx]);
	}
}
//----------------------------Software Controlled Clock Modulation-----------------------------------------------
/*struct clock_mod{
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
*/
void dump_clock_mod(struct clock_mod *s, FILE *writeFile)
{
	double percent = 0.0;
	if(s->duty_cycle == 0)
	{
		percent = 6.25;
	}
	else if (s->duty_cycle == 1)
	{
		percent = 12.5;
	}
	else if (s->duty_cycle == 2)
	{
		percent = 25.0;
	}
	else if (s->duty_cycle == 3)
	{
		percent = 37.5;
	}
	else if (s->duty_cycle == 4)
	{
		percent = 50.0;
	}
	else if (s->duty_cycle == 5)
	{
		percent = 63.5;
	}
	else if (s->duty_cycle == 6)
	{
		percent = 75.0;
	}
	else if (s->duty_cycle == 7)
	{
		percent = 87.5;
	}
	fprintf(writeFile, "duty_cycle 		= %d	\npercentage\t\t= %.2f\n", s->duty_cycle, percent);
	fprintf(writeFile, "duty_cycle_enable	= %d\n", s->duty_cycle_enable);
	fprintf(writeFile, "\n");
}

void get_clock_mod(int socket, int core, struct clock_mod *s)
{
	read_msr_by_coord(socket, core, 0, IA32_CLOCK_MODULATION, &(s->raw));
	//s->raw = 64; // temp value
	s->duty_cycle = MASK_VAL(s->raw, 3, 1);			// specific encoded values for target duty cycle

	s->duty_cycle_enable = MASK_VAL(s->raw, 4, 4);		// On-Demand Clock Modulation Enable
								// 1 = enabled, 0 disabled
}

int set_clock_mod(int socket, int core, struct clock_mod *s)
{
	uint64_t msrVal;
	read_msr_by_coord(socket, core, 0, IA32_CLOCK_MODULATION, &msrVal);
	//msrVal = 64; // temp value
    // replaced asserts with these two conditionals
    if (!(s->duty_cycle > 0 && s->duty_cycle < 8))
    {
        return -1;
    }
    if (!(s->duty_cycle_enable == 0 || s->duty_cycle_enable == 1))
    {
        return -1;
    }

	msrVal = (msrVal & (~(3<<1))) | (s->duty_cycle << 1);
	msrVal = (msrVal & (~(1<<4))) | (s->duty_cycle_enable << 4);

	write_msr_by_coord(socket, core, 0, IA32_CLOCK_MODULATION, msrVal);
    return 0;
}

//---------------------------------END CLOCK MODULATION FUNCTIONS-----------------------------------------------------------

