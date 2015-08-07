/* msr_turbo.h
 *
 * Low-level msr interface.
 *
 * Copyright (c) 2013-2015, Lawrence Livermore National Security, LLC.  
 * Produced at the Lawrence Livermore National Laboratory  
 * Written by Barry Rountree, rountree@llnl.gov
 *            Scott Walker,   walker91@llnl.gov
 *            Kathleen Shoga, shoga1@llnl.gov
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
 */

#ifndef MSR_TURBO_H
#define MSR_TURBO_H

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

void enable_turbo();
void disable_turbo();
void dump_turbo(FILE * writeFile);

#ifdef __cplusplus
}
#endif

#endif //MSR_TURBO_H


