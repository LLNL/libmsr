/*
 * Copyright (c) 2013, Lawrence Livermore National Security, LLC.  
 * Produced at the Lawrence Livermore National Laboratory  
 * Written by Kathleen Shoga, shoga1@llnl.gov.
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
#include "msr_thermal.h"

// Two defines below from Barry Rountree 
#define MASK_RANGE(m,n) ((((uint64_t)1<<((m)-(n)+1))-1)<<(n))
#define MASK_VAL(x,m,n) (((uint64_t)(x)&MASK_RANGE((m),(n)))>>(n))

// Section 35.7 Table 35-11
// Or Section 35.1 Table 35.2
#define IA32_THERM_STATUS		(0x19C) //core scope
#define IA32_THERM_INTERRUPT		(0x19B) //core scope
#define IA32_PACKAGE_THERM_STATUS	(0x1B1) //package scope
#define IA32_PACKAGE_THERM_INTERRUPT	(0x1B2) //package scope
#define IA32_CLOCK_MODULATION		(0x19A) // If Hyper-Threading Technology enabled processors, 
						// The IA32_CLOCK_MODULATION register is duplicated
						// for each logical processor. 
						// Must have it enabled and the same for all logical
						// processors within the physical processor
#define IA32_MISC_ENABLE		(0x1A0) 
#define MSR_TEMPERATURE_TARGET		(0x1A2)	// unique scope (Noted in documentation but do no know 
						// what it means exactly)

//---------------------MSR_TEMPERATURE_TARGET functions--------------------------------------------------------
/*
struct msr_temp_target{
	uint64_t raw;
	uint64_t temp_target;	//Read only (probably the TCC Activation Temp)
};
*/
void dump_msr_temp_target(struct msr_temp_target *s)
{
	fprintf(stdout, "temp_target		= %lu\n", s->temp_target);	
}

void get_msr_temp_target(int socket, int core, struct msr_temp_target *s)
{
	read_msr_single_core(socket, core, MSR_TEMPERATURE_TARGET, &(s->raw));
	//s->raw = 64;
	s->temp_target = MASK_VAL(s->raw, 23, 16);	// Minimum temperature at which PROCHOT will
    							// be asserted in degree Celsius (Probably 
							// the TCC Activation Temperature)	
}

// There is no set function for this because it is read only

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
	int enhanced_Intel_SpeedStep_Tech_enable;	// Read/Write (Package scope)
	int enable_monitor_fsm;				// Read/Write (thread scope)
	int limit_CPUID_maxval;				// Read/Write (thread scope)
	int xTPR_message_disable;			// Read/Write (thread scope)
	int XD_bit_disable;				// Read/Write (thread scope)
	int turbo_mode_disable;				// Read/Write (package scope)
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

void get_misc_enable(int package, struct misc_enable *s)
{
	read_msr(package, IA32_MISC_ENABLE, &(s->raw));
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

void set_misc_enable(int package, struct misc_enable *s)
{
	uint64_t msrVal;
	read_msr(package, IA32_MISC_ENABLE, &msrVal);
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

	write_msr(package, IA32_MISC_ENABLE, msrVal);
}

//---------------------END Misc_Enable functions----------------------------------------------------------------


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
void dump_clock_mod(struct clock_mod *s)
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
	fprintf(stdout, "duty_cycle 		= %d	\npercentage\t\t= %.2f\n", s->duty_cycle, percent);
	fprintf(stdout, "duty_cycle_enable	= %d\n", s->duty_cycle_enable);
	fprintf(stdout, "\n");
}

void get_clock_mod(int socket, int core, struct clock_mod *s)
{
	read_msr_single_core(socket, core, IA32_CLOCK_MODULATION, &(s->raw));
	//s->raw = 64; // temp value
	s->duty_cycle = MASK_VAL(s->raw, 3, 1);			// specific encoded values for target duty cycle

	s->duty_cycle_enable = MASK_VAL(s->raw, 4, 4);		// On-Demand Clock Modulation Enable
								// 1 = enabled, 0 disabled
}

void set_clock_mod(int socket, int core, struct clock_mod *s)
{
	uint64_t msrVal;
	read_msr_single_core(socket, core, IA32_CLOCK_MODULATION, &msrVal);
	//msrVal = 64; // temp value
	assert(s->duty_cycle > 0 && s->duty_cycle <8);
	assert(s->duty_cycle_enable == 0 || s->duty_cycle_enable == 1);

	msrVal = (msrVal & (~(3<<1))) | (s->duty_cycle << 1);
	msrVal = (msrVal & (~(1<<4))) | (s->duty_cycle_enable << 4);

	write_msr_single_core(socket, core, IA32_CLOCK_MODULATION, msrVal);
}

//---------------------------------END CLOCK MODULATION FUNCTIONS-----------------------------------------------------------

//---------------------------------Thermal Functions (status and interrupts)------------------------------------------------
/*
struct therm_stat{
	uint64_t raw;
	int status;			//Read only

	int status_log;			//Read and Write (Sticky bit)  
					//Cleared by software to 0 or RESET

	int PROCHOT_or_FORCEPR_event;	//Read only

	int PROCHOT_or_FORCEPR_log;	//Read and Write

	int crit_temp_status;		//Read only

	int crit_temp_log;		//Read and Write
					//Sticky bit

	int therm_thresh1_status;	//Read only

	int therm_thresh1_log;		//Read and Write 
					//Sticky bit

	int therm_thresh2_status;	//Read only

	int therm_thresh2_log;		//Read and Write
					//Sticky bit

	int power_limit_status;		//Read only

	int power_notification_log;	//Read and Write
					//Sticky bit

	int readout;			//Read only

	int resolution_deg_celsius;	//Read only

	int readout_valid;		//Read only
};

struct therm_interrupt{
	uint64_t raw;
	//Below all read and write
	int high_temp_enable;	
	int low_temp_enable;
	int PROCHOT_enable;
	int FORCEPR_enable;
	int crit_temp_enable;
	int thresh1_val;
	int thresh1_enable;
	int thresh2_val;
	int thresh2_enable;
	int pwr_limit_notification_enable;
};

struct pkg_therm_stat{
	uint64_t raw;
	int status;		//Read only

	int status_log;		//Read or Write Sticky bit (default: clear) 
				//Cleared by software to 0 or reset

	int PROCHOT_event;	//Read only

	int PROCHOT_log;	//Read or Write
				//Sticky bit

	int crit_temp_status;	//Read only
	int crit_temp_log;	//Read or write
				//Sticky bit

	int therm_thresh1_status;	//Read only
	int therm_thresh1_log;		//Read or write
					//Sticky bit

	int therm_thresh2_status;	//Read only
	int therm_thresh2_log;		//Read or write
					//Sticky bit

	int power_limit_status;		//Read only
	int power_notification_log;	//Read or write
					//Sticky bit

	int readout;			//Read only
};

struct pkg_therm_interrupt{
	uint64_t raw;
	//All read and write below
	int high_temp_enable;	
	int low_temp_enable;	
	int PROCHOT_enable;	
	int crit_temp_enable;	
	int thresh1_val;	
	int thresh1_enable;	
	int thresh2_val;	
	int thresh2_enable;
	int pwr_limit_notification_enable;
};
*/
void dump_therm_stat(int socket, int core, struct therm_stat * s)
{
	struct msr_temp_target x;
	get_msr_temp_target(socket, core, &x);
	int actTemp = x.temp_target - s->readout;
	fprintf(stdout, "status				= %d\n", s->status);
	fprintf(stdout, "status_log			= %d\n", s->status_log);
	fprintf(stdout, "PROCHOT_or_FORCEPR_event	= %d\n", s->PROCHOT_or_FORCEPR_event);
	fprintf(stdout, "PROCHOT_or_FORCEPR_log		= %d\n", s->PROCHOT_or_FORCEPR_log);
	fprintf(stdout, "crit_temp_status		= %d\n", s->crit_temp_status);
	fprintf(stdout, "crit_temp_log			= %d\n", s->crit_temp_log);
	fprintf(stdout, "therm_thresh1_status		= %d\n", s->therm_thresh1_status);
	fprintf(stdout, "therm_thresh1_log		= %d\n", s->therm_thresh1_log);
	fprintf(stdout, "therm_thresh2_status		= %d\n", s->therm_thresh2_status);
	fprintf(stdout, "therm_thresh2_log		= %d\n", s->therm_thresh2_log);
	fprintf(stdout, "power_limit_status		= %d\n", s->power_limit_status);
	fprintf(stdout, "power_notification_log		= %d\n", s->power_notification_log);
	fprintf(stdout, "readout				= %d\n", s->readout);
	fprintf(stdout, "Actual temperature:		= %d degrees Celsius\n", actTemp);
	fprintf(stdout, "resolution_deg_celsius		= %d\n", s->resolution_deg_celsius);
	fprintf(stdout, "readout_valid			= %d\n", s->readout_valid);
	fprintf(stdout, "\n");
}

void dump_therm_interrupt(int socket, int core, struct therm_interrupt *s)
{
	struct msr_temp_target x;
	get_msr_temp_target(socket, core, &x);
	int actTemp1 = x.temp_target - s->thresh1_val;
	int actTemp2 = x.temp_target - s->thresh2_val;
	fprintf(stdout, "high_temp_enable		= %d\n", s->high_temp_enable);
	fprintf(stdout, "low_temp_enable			= %d\n", s->low_temp_enable);
	fprintf(stdout, "PROCHOT_enable			= %d\n", s->PROCHOT_enable);
	fprintf(stdout, "FORCEPR_enable			= %d\n", s->FORCEPR_enable);
	fprintf(stdout, "crit_temp_enable		= %d\n", s->crit_temp_enable);
	fprintf(stdout, "thresh1_val			= %d\n", s->thresh1_val);
	fprintf(stdout, "Thresh1 actual temperature	= %d degrees Celsius\n", actTemp1);
	fprintf(stdout, "thresh1_enable			= %d\n", s->thresh1_enable);
	fprintf(stdout, "thresh2_val			= %d\n", s->thresh2_val);
	fprintf(stdout, "Thresh1 actual temperature	= %d degrees Celsius\n", actTemp2);
	fprintf(stdout, "thresh2_enable			= %d\n", s->thresh2_enable);
	fprintf(stdout, "pwr_limit_notification_enable	= %d\n", s->pwr_limit_notification_enable);
	fprintf(stdout, "\n");
}

void dump_pkg_therm_stat(int package, struct pkg_therm_stat * s)
{
	struct msr_temp_target x;
	get_msr_temp_target(package, 0, &x);	// use core 0 of the specified package 
						// hopefully the PTCC temp is the same regardless
						// of which core we choose
	int actTemp = x.temp_target - s->readout;
	fprintf(stdout, "status			= %d\n", s->status);
        fprintf(stdout, "status_log		= %d\n", s->status_log);
        fprintf(stdout, "PROCHOT_event		= %d\n", s->PROCHOT_event);
	fprintf(stdout, "PROCHOT_log		= %d\n", s->PROCHOT_log);
        fprintf(stdout, "crit_temp_status	= %d\n", s->crit_temp_status);
        fprintf(stdout, "crit_temp_log		= %d\n", s->crit_temp_log);
	fprintf(stdout, "therm_thresh1_status	= %d\n", s->therm_thresh1_status);
        fprintf(stdout, "therm_thresh1_log	= %d\n", s->therm_thresh1_log);
        fprintf(stdout, "therm_thresh2_status	= %d\n", s->therm_thresh2_status);
	fprintf(stdout, "therm_thresh2_log	= %d\n", s->therm_thresh2_log);
	fprintf(stdout, "power_limit_status	= %d\n", s->power_limit_status);
	fprintf(stdout, "power_notification_log	= %d\n", s->power_notification_log);	
	fprintf(stdout, "readout			= %d\n", s->readout);
	fprintf(stdout, "Actual Temperature	= %d degrees Celsius\n", actTemp);
	fprintf(stdout, "\n");
}

void dump_pkg_therm_interrupt(int package, struct pkg_therm_interrupt *s)
{
	struct msr_temp_target x;
	get_msr_temp_target(package, 0, &x);	// see comment in similar case above
	int actTemp1 = x.temp_target - s->thresh1_val;
	int actTemp2 = x.temp_target - s->thresh2_val;
	fprintf(stdout, "high_temp_enable		= %4d\n", s->high_temp_enable);
	fprintf(stdout, "low_temp_enable			= %4d\n", s->low_temp_enable);
	fprintf(stdout, "PROCHOT_enable			= %4d\n", s->PROCHOT_enable);
	fprintf(stdout, "crit_temp_enable		= %4d\n", s->crit_temp_enable);
	fprintf(stdout, "thresh1_val			= %4d\n", s->thresh1_val);
	fprintf(stdout, "Threshold1 actual temperature	= %4d degrees Celsius\n", actTemp1);
	fprintf(stdout, "thresh1_enable			= %4d\n", s->thresh1_enable);
	fprintf(stdout, "thresh2_val			= %4d\n", s->thresh2_val);
	fprintf(stdout, "Threshold2 actual temperature	= %4d degrees Celsius\n", actTemp2);
	fprintf(stdout, "thresh2_enable			= %4d\n", s->thresh2_enable);
	fprintf(stdout, "pwr_limit_notification_enable	= %4d\n", s->pwr_limit_notification_enable);
	fprintf(stdout, "\n");
}

void get_therm_stat(int socket, int core, struct therm_stat *s)
{
	read_msr_single_core(socket, core, IA32_THERM_STATUS, &(s->raw));
	//s->raw = 56879; //in dec
	s->status = MASK_VAL(s->raw, 0,0);			// Indicates whether the digital thermal sensor
								// high-temperature output signal (PROCHOT#) is
								// currently active.
								// (1=active)

	s->status_log = MASK_VAL(s->raw, 1,1);			// Indicates the history of the thermal sensor high
								// temperature output signal (PROCHOT#)
								// If equals 1, PROCHOT# has been asserted since a 
								// previous RESET or clear 0 by user
								
	s->PROCHOT_or_FORCEPR_event = MASK_VAL(s->raw, 2,2);	// Indicates whether PROCHOT# or FORCEPR# is being 
								// asserted by another agent on the platform

	s->PROCHOT_or_FORCEPR_log = MASK_VAL(s->raw, 3, 3);	// Indicates whether PROCHOT# or FORCEPR# has been 
								// asserted by aother agent on the platform since
								// the last clearing of the bit or a reset.
								// (1=has been externally asserted)
								// (0 to clear)
								// External PROCHOT# assertions are only acknowledged
								// if the Bidirectional Prochot feature is enabled
	
	s->crit_temp_status = MASK_VAL(s->raw, 4, 4);		// Indicates whether actual temp is currently higher
								// than or equal to the value set in Thermal Thresh 1
								// (0 then actual temp is lower)
								// (1 then equal to or higher)
	
	s->crit_temp_log = MASK_VAL(s->raw, 5, 5);		// Sticky bit indicates whether the crit temp detector
								// output signal has been asserted since the last reset
								// or clear 
								// (0 cleared) (1 asserted)

	s->therm_thresh1_status = MASK_VAL(s->raw, 6,6);	// Indicates whether actual temp is currently higher than 
								// or equal to the value set in Thermal Threshold 1
								// (0 actual temp is lower) 
								// (1 actual temp is greater than or equal to TT#1)
	
	s->therm_thresh1_log = MASK_VAL(s->raw, 7, 7);		// Sticky bit indicates whether Thermal Threshold #1 has 
								// been reached since last reset or clear 0. 
	
	s->therm_thresh2_status = MASK_VAL(s->raw, 8,8);	// Same as therm_thresh1_status, except for Thermal Threshold #2
	s->therm_thresh2_log = MASK_VAL(s->raw, 9, 9);		// Same as therm_thresh1_log, except for Thermal Threshold #2

	s->power_limit_status = MASK_VAL(s->raw, 10, 10);	// Indicates whether the processor is currently operating below
								// OS-requested P-state (specified in IA32_PERF_CTL), or 
								// OS-requested clock modulation duty cycle (in IA32_CLOCK_MODULATION)
								// This field supported only if CPUID.06H:EAX[bit 4] = 1
								// Package level power limit notification can be delivered 
								// independently to IA32_PACKAGE_THERM_STATUS MSR
	
	s->power_notification_log = MASK_VAL(s->raw, 11, 11);	// Sticky bit indicates the processor went below OS-requested P-state
								// or OS-requested clock modulation duty cycle since last RESET
								// or clear 0. Supported only if CPUID.06H:EAX[bit 4] = 1
								// Package level power limit notification is indicated independently
								// in IA32_PACKAGE_THERM_STATUS MSR

	s->readout = MASK_VAL(s->raw, 22, 16);			// Digital temperature reading in 1 degree Celsius relative to the 
								// TCC activation temperature
								// (0: TCC Activation temperature)
								// (1: (TCC Activation -1)... etc. )

	s->resolution_deg_celsius = MASK_VAL(s->raw, 30, 27);	// Specifies the resolution (tolerance) of the digital thermal
								// sensor. The value is in degrees Celsius. Recommended that
								// new threshold values be offset from the current temperature by 
								// at least the resolution + 1 in order to avoid hysteresis of
								// interrupt generation

	s->readout_valid = MASK_VAL(s->raw, 31, 31);		// Indicates if the digital readout is valid (valid if = 1)
}

void get_therm_interrupt(int socket, int core, struct therm_interrupt *s)
{
	read_msr_single_core(socket, core, IA32_THERM_INTERRUPT, &(s->raw));
	//s->raw = 64;	
	s->high_temp_enable = MASK_VAL(s->raw, 0, 0);		// Allows the BIOS to enable the generation of an inerrupt on the
								// transition from low-temp to a high-temp threshold. 
								// 0 (default) disables interrupts (1 enables interrupts)

	s->low_temp_enable = MASK_VAL(s->raw, 1, 1);		// Allows the BIOS to enable generation of an interrupt on the
								// transistion from high temp to low temp (TCC de-activation)
								// 0 (default) disables interrupts (1 enables interrupts)

	s->PROCHOT_enable = MASK_VAL(s->raw, 2, 2);		// Allows BIOS or OS to enable generation of an interrupt when 
								// PROCHOT has been asserted by another agent on the platform
								// and the Bidirectional Prochot feature is enabled
								// (0 disables) (1 enables)

	s->FORCEPR_enable = MASK_VAL(s->raw, 3, 3);		// Allows the BIOS or OS to enable generation of an interrupt when
								// FORCEPR # has been asserted by another agent on the platform. 
								// (0 disables the interrupt) (2 enables)

	s->crit_temp_enable = MASK_VAL(s->raw, 4, 4);		// Enables generations of interrupt when the critical temperature 
								// detector has detected a critical thermal condition. 
								// Recommended responce: system shutdown
								// (0 disables interrupt) (1 enables)

	s->thresh1_val = MASK_VAL(s->raw, 14, 8);		// A temp threshold. Encoded relative to the TCC Activation temperature 
								// (same format as digital readout) 
								// used to generate therm_thresh1_status and therm_thresh1_log and
								// Threshold #1 thermal interrupt delivery

	s->thresh1_enable = MASK_VAL(s->raw, 15, 15);		// Enables generation of an interrupt when the actual temperature  
								// crosses Threshold #1 setting in any direction 
								// (ZERO ENABLES the interrupt) (ONE DISABLES the interrupt)

	s->thresh2_val = MASK_VAL(s->raw, 22, 16);		// See above description for thresh1_val (just for thresh2)
	s->thresh2_enable = MASK_VAL(s->raw, 23, 23);		// See above description for thresh1_enable (just for thresh2)

	s->pwr_limit_notification_enable = MASK_VAL(s->raw, 24, 24);	// Enables generation of power notification events when the processor
									// went below OS-requested P-state or OS-requested clock modulation 
									// duty cycle. 
									// THIS FIELD SUPPORTED ONLY IF CPUID.06H:EAX[bit 4] = 1
									// Package level power limit notification can be enabled independently
									// by IA32_PACKAGE_THERM_INTERRUPT MSR
}

void get_pkg_therm_stat(int package, struct pkg_therm_stat *s)
{
	read_msr( package, IA32_PACKAGE_THERM_STATUS, &(s->raw) ); 
	//s->raw = 56879; //in dec
	s->status = MASK_VAL(s->raw,0,0); 			// Indicates whether the digital thermal sensor 
								// high-temp output signal (PROCHOT#) for the pkg
								// currently active. (1=active)

	s->status_log = MASK_VAL(s->raw,1,1);			// Indicates the history of thermal sensor high
       								// temp output signal (PROCHOT#) of pkg. 
								// (1= pkg PROCHOT# has been asserted since previous 
								// reset or last time software cleared bit.

	s->PROCHOT_event = MASK_VAL(s->raw,2,2);		// Indicates whether pkg PROCHOT# is being asserted by
								// another agent on the platform

	s->PROCHOT_log = MASK_VAL(s->raw,3,3);			// Indicates whether pkg PROCHET# has been asserted by 
								// another agent on the platform since the last clearing
								// of the bit by software or reset. (1= has been externally
								// asserted) (write 0 to clear)

	s->crit_temp_status = MASK_VAL(s->raw,4,4);		// Indicates whether pkg crit temp detector output signal
								// is currently active (1=active)

	s->crit_temp_log = MASK_VAL(s->raw,5,5);		// Indicates whether pkg crit temp detector output signal
       								//been asserted since the last clearing of bit or reset 
								//(1=has been asserted) (set 0 to clear)

	s->therm_thresh1_status = MASK_VAL(s->raw,6,6);		// Indicates whether actual pkg temp is currently higher 
								// than or equal to value set in Package Thermal Threshold #1
								// (0=actual temp lower) (1= actual temp >= PTT#1)

	s->therm_thresh1_log = MASK_VAL(s->raw,7,7);		// Indicates whether pkg therm threshold #1 has been reached 
								// since last software clear of bit or reset. (1= reached)
								// (clear with 0)

	s->therm_thresh2_status = MASK_VAL(s->raw,8,8);		// Same as above (therm_thresh1_stat) except it is for threshold #2
	s->therm_thresh2_log = MASK_VAL(s->raw,9,9);		// Same as above (therm_thresh2_log) except it is for treshold #2

	s->power_limit_status = MASK_VAL(s->raw,10,10);		// Indicates pkg power limit forcing 1 or more processors to  
								// operate below OS-requested P-state
								// (Note: pkg power limit violation may be caused by processor
								// cores or by devices residing in the uncore - examine 
								// IA32_THERM_STATUS to determine if cause from processor core)

	s->power_notification_log = MASK_VAL(s->raw,11,11);	// Indicates any processor from package went below OS-requested
								// P-state or OS-requested clock modulation duty cycle since
								// last clear or RESET

	s->readout = MASK_VAL(s->raw,22,16); 			// Pkg digital temp reading in 1 degree Celsius relative to
								// the pkg TCC activation temp
								// (0 = Package TTC activation temp)
								// (1 = (PTCC Activation - 1) etc. 
								// Note: lower reading actually higher temp
}

void set_therm_stat(int socket, int core, struct therm_stat *s)
{
	uint64_t msrVal;
	read_msr_single_core(socket, core, IA32_THERM_STATUS, &msrVal);
	//msrVal=64; //temp value without the read_msr_single_core
	assert(s->status_log == 0 || s->status_log == 1);
	assert(s->PROCHOT_or_FORCEPR_log == 0 || s->PROCHOT_or_FORCEPR_log == 1);
	assert(s->crit_temp_log == 0 || s->crit_temp_log == 1);
	assert(s->therm_thresh1_log == 0 || s->therm_thresh1_log == 1);
	assert(s->therm_thresh2_log == 0 || s->therm_thresh2_log == 1);
	assert(s->power_notification_log == 0 || s->power_notification_log == 1);
	
	msrVal = (msrVal & (~(1<<1))) | (s->status_log << 1);
	msrVal = (msrVal & (~(1<<3))) | (s->PROCHOT_or_FORCEPR_log << 1);
	msrVal = (msrVal & (~(1<<5))) | (s->crit_temp_log << 1);
	msrVal = (msrVal & (~(1<<7))) | (s->therm_thresh1_log << 1);
	msrVal = (msrVal & (~(1<<9))) | (s->therm_thresh2_log << 1);
	msrVal = (msrVal & (~(1<<11))) | (s->power_notification_log << 1);

	write_msr_single_core(socket, core, IA32_THERM_STATUS, msrVal);
}

void set_therm_interrupt(int socket, int core, struct therm_interrupt *s)
{
	uint64_t msrVal;
	read_msr_single_core(socket, core, IA32_THERM_INTERRUPT, &msrVal);
	//msrVal = 64; // temp value without read
	assert(s->high_temp_enable == 0 || s->high_temp_enable == 1);
	assert(s->low_temp_enable == 0 || s->low_temp_enable == 1);
	assert(s->PROCHOT_enable == 0 || s->PROCHOT_enable == 1);
	assert(s->FORCEPR_enable == 0 || s->FORCEPR_enable == 1);
	assert(s->crit_temp_enable == 0 || s->crit_temp_enable == 1);
	assert(s->thresh1_enable == 0 || s->thresh1_enable == 1);
	assert(s->thresh2_enable == 0 || s->thresh2_enable == 1);
	assert(s->pwr_limit_notification_enable == 0 || s->pwr_limit_notification_enable == 1);

	msrVal = (msrVal & (~(1<<0))) | (s->high_temp_enable << 0);
	msrVal = (msrVal & (~(1<<1))) | (s->low_temp_enable << 1);
	msrVal = (msrVal & (~(1<<2))) | (s->PROCHOT_enable << 2);
	msrVal = (msrVal & (~(1<<3))) | (s->FORCEPR_enable << 3);
	msrVal = (msrVal & (~(1<<4))) | (s->crit_temp_enable << 4);
	msrVal = (msrVal & (~(7<<8))) | (s->thresh1_val << 8);
	msrVal = (msrVal & (~(1<<15))) | (s->thresh1_enable << 15);
	msrVal = (msrVal & (~(7<<16))) | (s->thresh2_val << 16);
	msrVal = (msrVal & (~(1<<23))) | (s->thresh2_enable << 23);
	msrVal = (msrVal & (~(1<<24))) | (s->pwr_limit_notification_enable << 24);

	write_msr_single_core(socket, core, IA32_THERM_INTERRUPT, msrVal);
}

void set_pkg_therm_stat(int package, struct pkg_therm_stat *s)
{
	uint64_t msrVal;
	read_msr(package, IA32_PACKAGE_THERM_INTERRUPT, &msrVal);
	//msrVal=64; //temp value without the read_msr
	assert(s->status_log == 0 || s->status_log == 1);
	assert(s->PROCHOT_log == 0 || s->PROCHOT_log == 1);
	assert(s->crit_temp_log == 0 || s->PROCHOT_log == 1);
	assert(s->therm_thresh1_log == 0 || s->therm_thresh1_log == 1);
	assert(s->therm_thresh2_log == 0 || s->therm_thresh2_log == 1);
	assert(s->power_notification_log == 0 || s->power_notification_log == 1);
	
	msrVal = (msrVal & (~(1<<1))) | (s->status_log << 1);
	msrVal = (msrVal & (~(1<<3))) | (s->PROCHOT_log << 3);
	msrVal = (msrVal & (~(1<<5))) | (s->crit_temp_log << 5);
	msrVal = (msrVal & (~(1<<7))) | (s->therm_thresh1_log << 7);
	msrVal = (msrVal & (~(1<<9))) | (s->therm_thresh2_log << 9);
	msrVal = (msrVal & (~(1<<11))) | (s->power_notification_log << 11);
	write_msr(package, IA32_PACKAGE_THERM_INTERRUPT, msrVal);
}

void get_pkg_therm_interrupt(int package, struct pkg_therm_interrupt *s)
{
	read_msr( package, IA32_PACKAGE_THERM_INTERRUPT, &(s->raw));
	//s->raw = 56879;
	s->high_temp_enable = MASK_VAL(s->raw, 0, 0);	// Allows the BIOS to enable the generation of an interrupt on transition
							// from low temp to pkg high temp threshold
							// (0 (default)- disables interrupts) (1=enables interrupts)

	s->low_temp_enable = MASK_VAL(s->raw, 1, 1);	// Allows BIOS to enable the generation of an interrupt on transition
							// from high temp to a low temp (TCC de-activation)
							// (0 (default)- diabales interrupts) (1=enables interrupts)

	s->PROCHOT_enable = MASK_VAL(s->raw, 2, 2);	// Allows BIOS or OS to enable generation of an interrupt when pkg PROCHOT#
							// has been asserted by another agent on the platform and the Bidirectional
							// Prochot feature is enabled. (0 disables interrupt) (1 enables interrupt)

	s->crit_temp_enable = MASK_VAL(s->raw, 4, 4);	// Enables generation of interrupt when pkg crit temp detector has detected
							// a crit thermal condition. Recommended response: system shut down.
							// (0 disables interrupt) (1 enables)
							
	s->thresh1_val = MASK_VAL(s->raw, 14, 8);	// A temp threshold, encoded relative to the Package TCC Activation temp
							// using format as Digital Readout
							// Compared against the Package Digital Readout and used to generate 
							// Package Thermal Threshold #1 status and log bits as well as 
							// the Package Threshold #1 thermal interrupt delivery
	s->thresh1_enable = MASK_VAL(s->raw, 15, 15);	// Enables the generation of an interrupt when the actual temp crosses 
							// the thresh1_val setting in any direction
							// (0 enables interrupt) (1 disables interrupt)
							
	s->thresh2_val = MASK_VAL(s->raw, 22, 16);	// See thresh1_val
	s->thresh2_enable = MASK_VAL(s->raw, 23, 23);	// See thresh1_enable

	s->pwr_limit_notification_enable = MASK_VAL(s->raw, 24, 24);	// Enables generation of package power notification events
}

void set_pkg_therm_interrupt(int package, struct pkg_therm_interrupt *s)
{
	uint64_t msrVal;
	read_msr(package, IA32_PACKAGE_THERM_INTERRUPT, &msrVal);
	//msrVal=64; //temp value without the read_msr
	
	assert(s->high_temp_enable == 0 || s->high_temp_enable == 1);
	assert(s->low_temp_enable == 0 || s->low_temp_enable == 1);
	assert(s->PROCHOT_enable == 0 || s->PROCHOT_enable == 1);
	assert(s->crit_temp_enable == 0 || s->crit_temp_enable == 1);
	assert(s->thresh1_enable == 0 || s->thresh1_enable == 1);
	assert(s->thresh2_enable == 0 || s->thresh2_enable == 1);
	assert(s->pwr_limit_notification_enable == 0 || s->pwr_limit_notification_enable == 1);

	msrVal = (msrVal & (~(1<<0))) | (s->high_temp_enable << 0);
	msrVal = (msrVal & (~(1<<1))) | (s->low_temp_enable << 1);
	msrVal = (msrVal & (~(1<<2))) | (s->PROCHOT_enable << 2);
	msrVal = (msrVal & (~(1<<4))) | (s->crit_temp_enable << 4);
	msrVal = (msrVal & (~(7<<8))) | (s->thresh1_val << 8);	
	msrVal = (msrVal & (~(1<<15))) | (s->thresh1_enable << 15);
	msrVal = (msrVal & (~(7<<16))) | (s->thresh2_val << 16);
	msrVal = (msrVal & (~(1<<23))) | (s->thresh2_enable << 23);
	msrVal = (msrVal & (~(1<<24))) | (s->pwr_limit_notification_enable << 24);
	
	write_msr(package, IA32_PACKAGE_THERM_INTERRUPT, msrVal);
}

void Human_Interface_set_pkg_therm_interrupt(int package, struct pkg_therm_interrupt *s)
{
	int choice;
	int setValue;
	uint64_t setValue64;
	uint64_t msrVal;
	printf("Which would you like to set?\n");
	printf("(1) High-Temperature Interrupt Enable\n(2) Low-Temperature Interrupt Enable\n");
	printf("(3) PROCHOT# Interrupt Enable\n(4) Critical Temperature Interrupt Enable\n");
	printf("(5) Threshold 1 Value\n(6) Threshold 1 Interrupt Enable\n");
        printf("(7) Threshold 2 Value\n(8) Threshold 2 Interrupt Enable\n");	
	printf("(9) Package Power Limit Notification Enable\n");
	scanf("%d", &choice);
	while(getchar() != '\n') {;}
	if (choice != 5 && choice != 7)
	{
		printf("What value would you like to set? (0 or 1)\n");
		scanf("%d", &setValue);
		while(getchar() != '\n') {;}
	}
	else
	{
		int threshNum;
		if(choice == 5) { threshNum = 1;}
		else { threshNum = 2;}
		printf("How many degrees Celsius below the Package TCC Activation temperature");
		printf("\nwould you like to set as Threshold %d?\n", threshNum);
		scanf("%lu", &setValue64);
		while(getchar() != '\n') {;}
	}
	//Read in original msrVal so can change and write
	//read_msr( package, IA32_PACKAGE_THERM_INTERRUPT, &msrVal);
	msrVal=64;

	if(choice == 1)
	{
		int checkVal = MASK_VAL(msrVal,0,0);
		if(setValue == 0 && checkVal == 1)
		{
			msrVal = msrVal - 1;
		}
		else if(setValue == 1 && checkVal == 0)
		{
			msrVal = msrVal + 1;
		}
		else if (setValue == 0 || setValue == 1)
		{	
			printf("The value is already set.\n");
			return;
		}
		else 
		{
			printf("Not a valid value.\n");
			return;
		}
	}
	else if (choice == 2)
	{
		 int checkVal = MASK_VAL(msrVal,1,1);
		 if(setValue == 0 && checkVal == 1)
                 {   
                         msrVal = msrVal - 2;
                 }   
                 else if(setValue == 1 && checkVal == 0) 
                 {   
                         msrVal = msrVal + 2;
                 }   
                 else if (setValue == 0 || setValue == 1)
                 {    
                         printf("The value is already set.\n");
                         return;
                 }   
                 else 
                 {   
                         printf("Not a valid value.\n");
                         return;
                 }   
	}
	else if (choice == 3)
	{
		int checkVal = MASK_VAL(msrVal,2,2);
                if(setValue == 0 && checkVal == 1)
                {   
                        msrVal = msrVal - 4;
                }   
                else if(setValue == 1 && checkVal == 0)  
                {   
                        msrVal = msrVal + 4;
                }   
                else if (setValue == 0 || setValue == 1)
                {
                        printf("The value is already set.\n");
                        return;
                }   
                else 
                {   
			printf("Not a valid value.\n");
                        return;
                }
	}
	else if (choice == 4)
	{
		int checkVal = MASK_VAL(msrVal,4,4);
		if(setValue == 0 && checkVal == 1)
                {   
                         msrVal = msrVal - 16;
                }   
                else if(setValue == 1 && checkVal == 0)  
                {   
                         msrVal = msrVal +16;
                }   
                else if (setValue == 0 || setValue == 1)
                { 
			 printf("The value is already set.\n");
                         return;
                }   
                else 
                {   
                         printf("Not a valid value.\n");
                         return;
                }
	}
	else if (choice == 5)
	{
		msrVal = (msrVal & (~(7<<8))) | (setValue64 << 8);
		printf("New Threshold 1 is %lu degrees from the PTCC Activation temperature.\n", MASK_VAL(msrVal, 14,8));
	}
	else if (choice == 6)
	{
		int checkVal = MASK_VAL(msrVal,15,15);
        	if(setValue == 0 && checkVal == 1)
        	{   
                	msrVal = msrVal - 32768;
        	}   
        	else if(setValue == 1 && checkVal == 0)  
        	{   
                	msrVal = msrVal + 32768;
                }   
                else if (setValue == 0 || setValue == 1)
                {       
			printf("The value is already set.\n");
			return;
                }   
                else 
                {   
                        printf("Not a valid value.\n");
                        return;
                }
	}
	else if (choice == 7)
	{
		msrVal = (msrVal & (~(7<<8))) | (setValue64 << 8);
		printf("New Threshold 2 is %lu degrees from the PTCC Activation temperature.\n", MASK_VAL(msrVal, 14,8));
	}
	else if (choice == 8)
	{
		int checkVal = MASK_VAL(msrVal,23,23);
		if(setValue == 0 && checkVal == 1)
		{
			msrVal = msrVal - 8388608;
		}
		else if (setValue == 1 && checkVal == 0)
		{
			msrVal = msrVal + 8388608;
		}
		else if (setValue == 0 || setValue == 1)
		{
			printf("The value is already set.\n");
			return;
		}
		else
		{
			printf("Not a valid value.\n");
			return;
		}

	}
	else if (choice == 9)
	{
		int checkVal = MASK_VAL(msrVal,24,24);
		if(setValue == 0 && checkVal == 1)
		{
			msrVal = msrVal - 16777216; //2^24
		}
		else if (setValue == 1 && checkVal == 0)
		{
			msrVal = msrVal + 16777216;
		}
		else if (setValue == 0 || setValue == 1)
		{
			printf("The value is already set.\n");
			return;
		}
		else
		{
			printf("Not a valid value.\n");
			return;
		}
	}
	else
	{
		printf("Not a valid choice.");
		return;
	}
	//write_msr(package, IA32_PACKAGE_THERM_INTERRUPT, msrVal);
	return;
}

void dump_core_temp(int socket, int core, struct therm_stat * s)
{
	struct msr_temp_target x;
        get_msr_temp_target(socket, core, &x);
        int actTemp = x.temp_target - s->readout;
	printf("QQQ %d %d %d", core, socket, actTemp);
}

//--------------------------------- END Thermal Functions --------------------------------------------------------------
/*
int main()
{
	init_msr();
	printf("\nTCC Activation Temp\n");
	struct msr_temp_target aa;
	get_msr_temp_target(1,1,&aa);
	dump_msr_temp_target(&aa);

	printf("\nMiscellaneous Enable\n");
	struct misc_enable a;
	get_misc_enable(1, &a);
	dump_misc_enable(&a);

	printf("\nClock Modulation\n");
	struct clock_mod b; 
	get_clock_mod(1,1,&b);
	dump_clock_mod(&b);

	printf("\nMiscellaneous Enable\n");
	struct misc_enable a;
	get_misc_enable(1,&a);
	dump_misc_enable(&a);	
	a.turbo_mode_disable = 1;
	a.XD_bit_disable = 1;
	printf("\n");
	set_misc_enable(1, &a);
	dump_misc_enable(&a);

	printf("\nClock Modulation\n");
	struct clock_mod b;
	get_clock_mod(1,1,&b);
	dump_clock_mod(&b);
	b.duty_cycle = 3;
	b.duty_cycle_enable = 1;
	set_clock_mod(1,1, &b);
	dump_clock_mod(&b);

	printf("\nThermal Interrupt\n");
	struct therm_interrupt c;
	get_therm_interrupt(1,1,&c);
	dump_therm_interrupt(&c);
	c.high_temp_enable = 1;
	c.low_temp_enable = 1;
	c.thresh1_val = 25;
	set_therm_interrupt(1,1,&c);
	dump_therm_interrupt(&c);


	printf("\nThermal Status\n");	
	struct therm_stat d;
	get_therm_stat(1,1,&d);
	dump_therm_stat(&d);
	d.status = 0;
	d.readout = 22;
	d.PROCHOT_or_FORCEPR_log = 0;
	set_therm_stat(1,1,&d);
	dump_therm_stat(&d);


	printf("\nPackage Thermal Interrupt\n");
	struct pkg_therm_interrupt e;
	get_pkg_therm_interrupt(1, &e);
	dump_pkg_therm_interrupt(&e);
	e.high_temp_enable = 0;
	set_pkg_therm_interrupt(1,&e);
	dump_pkg_therm_interrupt(&e);	
	
	printf("\nPackage Thermal Status:\n");
	struct pkg_therm_stat f;
	get_pkg_therm_stat(1,&f);
	dump_pkg_therm_stat(&f);
	f.status = 0;
	set_pkg_therm_stat(1, &f);
	dump_pkg_therm_stat(&f);
	
	finalize_msr();
	return 0;
}*/
