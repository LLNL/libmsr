/* msr_misc.h
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

#ifndef MSR_MISC_H
#define MSR_MISC_H
#include <stdio.h>
#include <stdint.h>
#include <sys/types.h>
#include "master.h"

struct misc_enable{
	uint64_t raw;
	__u8 fast_string_enable;			// Read/Write (thread scope)
	__u8 auto_TCC_enable;				// Read/Write
	__u8 performance_monitoring;			// Read	(thread scope)
	__u8 branch_trace_storage_unavail;		// Read only (thread scope)
	__u8 precise_event_based_sampling_unavail;	// Read Only (thread scope)
	__u8 TM2_enable;					// Read/Write
	__u8 enhanced_Intel_SpeedStep_Tech_enable;	// Read/Write (Package scope)
	__u8 enable_monitor_fsm;				// Read/Write (thread scope)
	__u8 limit_CPUID_maxval;				// Read/Write (thread scope)
	__u8 xTPR_message_disable;			// Read/Write (thread scope)
	__u8 XD_bit_disable;				// Read/Write (thread scope)
	__u8 turbo_mode_disable;				// Read/Write (package scope)
};

struct pkg_cres
{
    uint64_t ** pkg_c2;
    uint64_t ** pkg_c3;
    uint64_t ** pkg_c6;
    uint64_t ** pkg_c7;
};

struct core_cres
{
    uint64_t ** core_c3;
    uint64_t ** core_c6;
    uint64_t ** core_c7;
};

#ifdef __cplusplus
extern "C" {
#endif

void dump_misc_enable(struct misc_enable *s);
void get_misc_enable(unsigned socket, struct misc_enable *s);
void set_misc_enable(unsigned socket, struct misc_enable *s);
int pkg_cres_storage(struct pkg_cres ** pcr);
int core_cres_storage(struct core_cres ** ccr);
int dump_pkg_cres_label(FILE * writedest);
int dump_pkg_cres(FILE * writedest);
int dump_core_cres_label(FILE * writedest);
int dump_core_cres(FILE * writedest);

#ifdef __cplusplus
}
#endif
#endif // MSR_MISC_H
