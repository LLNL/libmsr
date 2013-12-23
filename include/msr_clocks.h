/*
 * Copyright (c) 2013, Lawrence Livermore National Security, LLC.  
 * Produced at the Lawrence Livermore National Laboratory  
 * Written by Barry Rountree, rountree@llnl.gov.
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

#ifndef MSR_CLOCKS_H
#define MSR_CLOCKS_H

#define NUM_PACKAGES 1

void read_aperf(int package, uint64_t *aperf); 
void read_mperf(int package, uint64_t *mperf); 
void read_tsc  (int package, uint64_t *tsc); 
double get_effective_frequency(int package);
void dump_clocks();

#endif //MSR_CLOCKS_H
