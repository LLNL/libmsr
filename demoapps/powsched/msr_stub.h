/* msr_stub.h
 *
 * Copyright (c) 2011-2015, Lawrence Livermore National Security, LLC. LLNL-CODE-645430
 * Produced at Lawrence Livermore National Laboratory  
 * Written by  Barry Rountree,   rountree@llnl.gov
 *             Daniel Ellsworth, ellsworth8@llnl.gov
 *             Scott Walker,     walker91@llnl.gov
 *             Kathleen Shoga,   shoga1@llnl.gov
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
/*
 * Stub for systems that don't have libmsr
 */
#ifndef MSR_STUB_H
#define MSR_STUB_H

#include <stdint.h>

struct rapl_data{
    uint64_t old_pkg_bits;
    uint64_t pkg_bits;

    uint64_t old_dram_bits;
    uint64_t dram_bits;

    double old_pkg_joules;
    double pkg_joules;

    double old_dram_joules;
    double dram_joules;

    struct timeval old_now;
    struct timeval now;

    double elapsed;
    double pkg_delta_joules;
    double pkg_watts;
    double dram_delta_joules;
    double dram_watts;


    uint64_t flags;
};

struct rapl_limit{
    double      watts;      // User-friendly interface.
    double      seconds;
    uint64_t    bits;       // User-unfriendly interface.
};

static inline void set_rapl_limit( const int socket, struct rapl_limit* limit1, struct rapl_limit* limit2, struct rapl_limit* dram ) {}
static inline void get_rapl_limit( const int socket, struct rapl_limit* limit1, struct rapl_limit* limit2, struct rapl_limit* dram ) {}

static inline void read_rapl_data( const int socket, struct rapl_data *r ) {}

#endif
