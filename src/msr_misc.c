/* msr_core.c
 *
 * Low-level msr interface.
 *
 * Copyright (c) 2015, Lawrence Livermore National Security, LLC.  
 * Produced at the Lawrence Livermore National Laboratory  
 * Written by Kathleen Shoga, shoga1@llnl.gov.
 * Modified by Scott Walker, walker91@llnl.gov
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
 */

#include <stdio.h>
#include <stdint.h>
#include <sys/time.h>
#include <stddef.h>
#include <assert.h>
#include "msr_core.h"
#include "msr_misc.h"

// Section 35.7 Table 35-11
// Or Section 35.1 Table 35.2
#define IA32_MISC_ENABLE		(0x1A0) 

//----------------------Misc_Enable functions-------------------------------------------------------------------
/*
struct misc_enable{
	uint64_t raw;
	int fast_string_enable;			// Read/Write (thread scope)	
	int auto_TCC_enable;				// Read/Write 
	int performance_monitoring;			// Read	(thread scope)
	int branch_trace_storage_unavail;		// Read only (thread scope)
	int precise_event_based_sampling_unavail;	// Read Only (thread scope)
	int TM2_enable;					// Read/Write 
	int enhanced_Intel_SpeedStep_Tech_enable;	// Read/Write (socket scope)
	int enable_monitor_fsm;				// Read/Write (thread scope)
	int limit_CPUID_maxval;				// Read/Write (thread scope)
	int xTPR_message_disable;			// Read/Write (thread scope)
	int XD_bit_disable;				// Read/Write (thread scope)
	int turbo_mode_disable;				// Read/Write (socket scope)
};
*/
void dump_misc_enable(struct misc_enable *s)
{
	fprintf(stdout, "fast_string_enable			= %d\n", s->fast_string_enable);
	fprintf(stdout, "auto_TCC_enable				= %d\n", s->auto_TCC_enable);
	fprintf(stdout, "performance_monitoring			= %d\n", s->performance_monitoring);
	fprintf(stdout, "branch_trace_storage_unavail		= %d\n", s->branch_trace_storage_unavail);
	fprintf(stdout, "precise_event_based_sampling_unavail	= %d\n", s->precise_event_based_sampling_unavail);
	fprintf(stdout, "TM2_enable				= %d\n", s->TM2_enable);
	fprintf(stdout, "enhanced_Intel_SpeedStep_Tech_enable	= %d\n", s->enhanced_Intel_SpeedStep_Tech_enable);
	fprintf(stdout, "enable_monitor_fsm			= %d\n", s->enable_monitor_fsm);
	fprintf(stdout, "limit_CPUID_maxval			= %d\n", s->limit_CPUID_maxval);
	fprintf(stdout, "xTPR_message_disable			= %d\n", s->xTPR_message_disable);
	fprintf(stdout, "XD_bit_disable				= %d\n", s->XD_bit_disable);
	fprintf(stdout, "turbo_mode_disable			= %d\n", s->turbo_mode_disable);
}

void get_misc_enable(unsigned socket, struct misc_enable *s)
{
    sockets_assert(&socket, __LINE__, __FILE__);
	read_msr_by_coord(socket, 0, 0, IA32_MISC_ENABLE, &(s->raw));
	//s->raw = 164;
    s->fast_string_enable = MASK_VAL(s->raw, 0, 0);				// When set, fast-strings feature (for REP MOVS and REP STORS) 
										// is enabled (default), (cleared = disabled)

    s->auto_TCC_enable = MASK_VAL(s->raw, 3, 3);				// 0 = disabled (default)
										// Note: clearing might be ignored in critical thermal
										// conditions. In this case, TM1, TM2, and adaptive 
										// thermal throttling will still be active. 
										// 
										// 1 = enables the thermal control circuit (TCC) portion
										// of the Intel Thermal Monitor feature. Allows the
										// processor to automatically reduce power consumption in 
										// response to TCC activation)
										
	s->performance_monitoring = MASK_VAL(s->raw, 7,7);			// 1 = performance monitoring enabled
       										// 0 = disabled

	s->branch_trace_storage_unavail = MASK_VAL(s->raw, 11, 11);		// 1 = Processor doesn't support branch trace storage (BTS)
										// 0 = BTS is supported

	s->precise_event_based_sampling_unavail = MASK_VAL(s->raw, 12, 12);	// 1 = PEBS is not supported
										// 0 = PEBS is supported

	s->TM2_enable = MASK_VAL(s->raw, 13, 13);				// 1 = TM2 enabled
										// 0 = TM2 disabled

	s->enhanced_Intel_SpeedStep_Tech_enable = MASK_VAL(s->raw, 16, 16);	// 1 = Enhanced Intel SpeedStep Technology enabled
										// 0 = disabled

	s->enable_monitor_fsm = MASK_VAL(s->raw, 18, 18);			// 0 = MONITOR feature flag is not set (CPUID.01H:ECX[bit 3]=0)
										// This indicates that MONITOR/MWAIT are not supported.
										// Software attempts to execute MONITOR/MWAIT will cause #UD
										// when this bit is 0.
										// 1 (default) = MONITOR/MWAIT are supported. (CPUID... = 1)
										//
										// If the SSE3 feature flag ECX[0] is not set
										// (CPUID.01H:ECX[bit 0] = 0), the OS must not attempt to 
										// alter this bit. BIOS must leave it in the default state.
										// Writing this bit when SSE3 feature flag is 0 may generate
										// a #GP exception

	s->limit_CPUID_maxval = MASK_VAL(s->raw, 22, 22);			// Before setting this bit, BIOS must execute the CPUID.0H
										// and examine the maximum value returned in EAX[7:0].
										// If the max is greater than 3, bit is supported.
										// Otherwise, this bit is not supported, and writing to this
										// bit when the max value is greater than 3 may generate
										// a #GP exception
										//
										// Setting this bit may cause unexpected behavior in software
										// that depends on the availability of CPUID leaves greater
										// than 3.
										//
										// 1 = CPUID.00H returns a max value in EAX[7:0] of 3.
										// BIOS should contain a setup question that allows users to
										// specify when the installed OS does not support CPUID 
										// functions greater than 3.
										
	s->xTPR_message_disable = MASK_VAL(s->raw, 23, 23);			// xTPR messages are optional messages that allow the 
										// processor to inform the chipset of its priority.
										// 1 = xTPR messages are disabled
										// 0 = enabled

	s->XD_bit_disable = MASK_VAL(s->raw, 34, 34);				// BIOS must not alter the contents of this bit location. If
										// XD is not supported, writing this bit to 1 when the XD Bit 
										// extended flag is set 0 may generate a #GP exception
										// 
										// 1 = Execute Disable Bit feature (XD bit) is disabled
										// and the XD bit extended feature flag will be clear
										// (CPUID.80000001H:EDX[20] = 0
										//
										// 0 (default) = the EX bit feature (if available) allows 
										// the OS to enable PAE paging and take advantage of data 
										// only pages
										
	s->turbo_mode_disable = MASK_VAL(s->raw, 38, 38);			// 1 (on processors that support Intel Turbo Boost Tech) =
										// turbo mode feature disabled and IDA_Enable feature flag 
										// will be clear (CPUID.06H: EAX[1]=0)
										//
										// 0 (on proccessors that support IDA) = CPUID.06H: EAX[1] 
										// reports the processor's support of turbo mode is enabled
										//
										// Note: the power on default value is used by BIOS to detect
										// hardware support of turbo mode. If power-on default is 1,
										// turbo mode is available in the processor. If 0, then turbo
										// mode is not available. 
}

void set_misc_enable(unsigned socket, struct misc_enable *s)
{
	uint64_t msrVal;
    sockets_assert(&socket, __LINE__, __FILE__);
	read_msr_by_coord(socket, 0, 0, IA32_MISC_ENABLE, &msrVal);
	//msrVal = 64; //temp value
	assert(s->fast_string_enable == 0 || s->fast_string_enable == 1);
	assert(s->auto_TCC_enable == 0 || s->auto_TCC_enable == 1);
	assert(s->TM2_enable == 0 || s->TM2_enable == 1);
	assert(s->enhanced_Intel_SpeedStep_Tech_enable == 0 || s->enhanced_Intel_SpeedStep_Tech_enable == 1);
	assert(s->limit_CPUID_maxval == 0 || s->limit_CPUID_maxval == 1);
	assert(s->xTPR_message_disable == 0 || s->xTPR_message_disable == 1);
	assert(s->XD_bit_disable == 0 || s->XD_bit_disable == 1);
	assert(s->turbo_mode_disable == 0 || s->turbo_mode_disable == 1);

	msrVal = (msrVal & (~(1<< 0))) | (s->fast_string_enable << 0);
	msrVal = (msrVal & (~(1<< 3))) | (s->auto_TCC_enable << 3);
	msrVal = (msrVal & (~(1<< 13))) | (s->TM2_enable << 13);
	msrVal = (msrVal & (~(1<< 16))) | (s->enhanced_Intel_SpeedStep_Tech_enable << 16);
	msrVal = (msrVal & (~(1<< 22))) | (s->limit_CPUID_maxval << 22);
	msrVal = (msrVal & (~(1<< 23))) | (s->xTPR_message_disable << 23);
	msrVal = (msrVal & (~((uint64_t)1<< (uint64_t)34))) | ((uint64_t)s->XD_bit_disable << (uint64_t)34);
	msrVal = (msrVal & (~((uint64_t)1<< (uint64_t)38))) | ((uint64_t)s->turbo_mode_disable << (uint64_t)38);

	write_msr_by_coord(socket, 0, 0, IA32_MISC_ENABLE, msrVal);
}

//---------------------END Misc_Enable functions----------------------------------------------------------------

