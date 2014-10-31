/* checkCPUID.h
 *
 * Copyright (c) 2011, 2012, 2013, 2014 by Lawrence Livermore National Security, LLC. LLNL-CODE-645430 
 * Produced at the Lawrence Livermore National Laboratory.
 * Written by Kathleen Shoga and Barry Rountree (shoga1|rountree@llnl.gov).
 * All rights reserved.
 *
 * This file is part of libmsr.
 *
 * libmsr is free software: you can redistribute it and/or 
 * modify it under the terms of the GNU Lesser General Public 
 * License as published by the Free Software Foundation, either 
 * version 3 of the License, or (at your option) any
 * later version.
 *
 * libmsr is distributed in the hope that it will be useful, but 
 * WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU 
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public 
 * License along
 * with libmsr. If not, see <http://www.gnu.org/licenses/>.
 *
 * This material is based upon work supported by the U.S. Department
 * of Energy's Lawrence Livermore National Laboratory. Office of 
 * Science, under Award number DE-AC52-07NA27344.
 *
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
