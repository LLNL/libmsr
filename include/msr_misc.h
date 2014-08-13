/* msr_misc.h
 * 
 * Copyright (c) 2011, 2012, 2013, 2014 by Lawrence Livermore National Security, LLC. LLNL-CODE-645430 
 * Written by Kathleen Shoga and Barry Rountree (shoga1|rountree@llnl.gov).
 * This file is part of libmsr and is licensed under the LGLP.  See the LICENSE file for details.
 */
#ifndef MSR_MISC_H
#define MSR_MISC_H
#include <stdio.h>

struct misc_enable{
	uint64_t raw;
	int fast_string_enable;			// Read/Write (thread scope)	
	int auto_TCC_enable;				// Read/Write 
	int performance_monitoring;			// Read	(thread scope)
	int branch_trace_storage_unavail;		// Read only (thread scope)
	int precise_event_based_sampling_unavail;	// Read Only (thread scope)
	int TM2_enable;					// Read/Write 
	int enhanced_Intel_SpeedStep_Tech_enable;	// Read/Write (Package scope)
	int enable_monitor_fsm;				// Read/Write (thread scope)
	int limit_CPUID_maxval;				// Read/Write (thread scope)
	int xTPR_message_disable;			// Read/Write (thread scope)
	int XD_bit_disable;				// Read/Write (thread scope)
	int turbo_mode_disable;				// Read/Write (package scope)
};

void dump_misc_enable(struct misc_enable *s);
void get_misc_enable(int package, struct misc_enable *s);
void set_misc_enable(int package, struct misc_enable *s);

#endif
