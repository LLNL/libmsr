/* checkCPUID.h
 *
 * Copyright (c) 2011, 2012, 2013, 2014 by Lawrence Livermore National Security, LLC. LLNL-CODE-645430 
 * Written by Kathleen Shoga and Barry Rountree (shoga1|rountree@llnl.gov).
 * This file is part of libmsr and is licensed under the LGLP.  See the LICENSE file for details.
 */
#ifndef CHECKCPUID_H
#define CHECKCPUID_H
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

bool 	cpuid_MPERF_and_APERF();
bool 	cpuid_timeStampCounter_avail();
int  	cpuid_PMC_num();
int  	cpuid_PERFEVTSEL_num();
bool 	cpuid_perf_global_ctrl_EN_PMC();
bool 	cpuid_perf_global_ctrl_EN_FIXED_CTRnum();
bool 	cpuid_misc_enable_turboBoost();
bool 	cpuid_misc_enable_xTPRmessageDisable();
bool 	cpuid_misc_enable_XDbitDisable();
bool 	cpuid_clock_mod_extended();
bool 	cpuid_therm_stat_therm_thresh();
bool 	cpuid_therm_stat_powerlimit();
bool 	cpuid_therm_stat_readout();
bool 	cpuid_therm_interrupt_powerlimit();
bool 	cpuid_pkg_therm_Stat_AND_Interrupt();
int32_t	cpuid_maxleaf();
void 	cpuid_printVendorID();
int 	cpuid_pkg_maxPhysicalProcessorCores();
int 	cpuid_pkg_maxLogicalProcessors();
int 	cpuid_num_fixed_perf_counters();
int 	cpuid_width_fixed_perf_counters();

#endif //CHECKCPUID_H
