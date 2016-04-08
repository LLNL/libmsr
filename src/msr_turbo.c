/* msr_turbo.c
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
#include <stdint.h>
#include "msr_core.h"
#include "msr_turbo.h"
#include "memhdlr.h"

/*
// MSRs common to 062A and 062D.
#define MSR_MISC_ENABLE			0x1A0	// aka IA32_MISC_ENABLE
						// setting bit 38 high DISABLES turbo mode. To be done in the BIOS.
						//
#define IA32_PERF_CTL			0x199   // setting bit 32 high DISABLES turbo mode. This is the software control.
*/

int turbo_storage(uint64_t *** val)
{
    static uint64_t ** perf_ctl = NULL;
    static int init = 1;
    static uint64_t numDevs = 0;
    if (init)
    {
        numDevs = num_devs();
        perf_ctl = (uint64_t **) libmsr_malloc(numDevs * sizeof(uint64_t *));
        init = 0;
    }
    if (val)
    {
        *val = perf_ctl;
    }
    return 0;
}

void
disable_turbo(){

	int j;
    static uint64_t numDevs = 0;
    static uint64_t ** val = NULL;
    if (!numDevs)
    {
        numDevs = num_devs();
        val = (uint64_t **) libmsr_malloc(numDevs * sizeof(uint64_t *));
        turbo_storage(&val);
    }
    // Set bit 32 "IDA/Turbo DISENGAGE" of IA32_PERF_CTL to 0.
    read_batch(PERF_CTL);
	for(j=0; j<numDevs; j++){
		*val[j] |= ((uint64_t)1) << 32;
	}
    write_batch(PERF_CTL);
}


void
enable_turbo(){
	int j;
    static uint64_t numDevs = 0;
    static uint64_t ** val =  NULL;
    if (!numDevs)
    {
        numDevs = num_devs();
        val = (uint64_t **) libmsr_malloc(numDevs * sizeof(uint64_t *));
        turbo_storage(&val);
    }
    // Set bit 32 "IDA/Turbo DISENGAGE" of IA32_PERF_CTL to 1.
    read_batch(PERF_CTL);
	for(j=0; j<numDevs;j++){
		*val[j] &= ~(((uint64_t)1) << 32);
        fprintf(stderr, "0x%016lx\t", *val[j] & (((uint64_t)1)<<32));
	}
    write_batch(PERF_CTL);
}

void
dump_turbo(FILE * writeFile){
	int core;
    static uint64_t numDevs = 0;
    if (!numDevs)
    {
        numDevs = num_devs();
    }
	uint64_t val;
    for(core=0; core<numDevs; core++){
        read_msr_by_idx(core, MSR_MISC_ENABLE, &val);
        fprintf(writeFile, "Core: %d\n", core);
        fprintf(writeFile, "0x%016lx\t", val & (((uint64_t)1)<<38));
        fprintf(writeFile, "| MSR_MISC_ENABLE | 38 (0=Turbo Available) \n");
        read_msr_by_idx(core, IA32_PERF_CTL, &val);
        fprintf(writeFile, "0x%016lx \t", val & (((uint64_t)1)<<32));
        fprintf(writeFile, "| IA32_PERF_CTL | 32 (0=Turbo Engaged) \n");
    }
}
