/* profile.h
 *
 * Low-level msr interface.
 *
 * Copyright (c) 2015, Lawrence Livermore National Security, LLC.  
 * Produced at the Lawrence Livermore National Laboratory  
 * Written by Barry Rountree, rountree@llnl.gov.
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

#ifndef MSR_PROFILE_H
#define MSR_PROFILE_H

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include "msr_core.h"
#include "msr_thermal.h"
#include "msr_rapl.h"
#include "msr_clocks.h"

#ifdef __cplusplus
extern "C" {
#endif

void msr_profile();

#ifdef __cplusplus
}
#endif

#endif // MSR_PROFILE_H
