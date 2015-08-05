/* msr_cpuid.h
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

#ifndef MSR_CHECKCPUID_H
#define MSR_CHECKCPUID_H
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

void cpuid_get_model(uint64_t * model);
void read_csr(uint64_t * val);
void cpuid_detect_core_conf(uint64_t * coresPerSocket, uint64_t * hyperThreads, uint64_t *sockets, int * HTenabled);
bool cpuid_MPERF_and_APERF();
bool cpuid_timeStampCounter_avail();
int cpuid_PMC_num();
int cpuid_PERFEVTSEL_num();
bool cpuid_perf_global_ctrl_EN_PMC();
bool cpuid_perf_global_ctrl_EN_FIXED_CTRnum();
bool cpuid_misc_enable_turboBoost();
bool cpuid_misc_enable_xTPRmessageDisable();
bool cpuid_misc_enable_XDbitDisable();
bool cpuid_clock_mod_extended();
bool cpuid_therm_stat_therm_thresh();
bool cpuid_therm_stat_powerlimit();
bool cpuid_therm_stat_readout();
bool cpuid_therm_interrupt_powerlimit();
bool cpuid_pkg_therm_Stat_AND_Interrupt();
uint64_t cpuid_maxleaf();
void cpuid_printVendorID();
int cpuid_pkg_maxPhysicalProcessorCores();
int cpuid_pkg_maxLogicalProcessors();
int cpuid_num_fixed_perf_counters();
int cpuid_width_fixed_perf_counters();

#ifdef __cplusplus
}
#endif

#endif // CHECKCPUID_H
