/*
 * Copyright (c) 2013-2017, Lawrence Livermore National Security, LLC.
 *
 * Produced at the Lawrence Livermore National Laboratory. Written by:
 *     Barry Rountree <rountree@llnl.gov>,
 *     Daniel Ellsworth <ellsworth8@llnl.gov>,
 *     Scott Walker <walker91@llnl.gov>, and
 *     Kathleen Shoga <shoga1@llnl.gov>.
 *
 * LLNL-CODE-645430
 *
 * All rights reserved.
 *
 * This file is part of libmsr. For details, see https://github.com/LLNL/libmsr.git.
 *
 * Please also read libmsr/LICENSE for our notice and the LGPL.
 *
 * libmsr is free software: you can redistribute it and/or modify it under the
 * terms of the GNU Lesser General Public License (as published by the Free
 * Software Foundation) version 2.1 dated February 1999.
 *
 * libmsr is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the IMPLIED WARRANTY OF MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the terms and conditions of the GNU Lesser
 * General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with libmsr; if not, write to the Free Software Foundation, Inc., 59
 * Temple Place, Suite 330, Boston, MA 02111-1307 USA.
 */

#include <stdlib.h>
#include <sys/types.h>

#include <msr_core.h>
#include <msr_rapl.h>

static struct rapl_data *rdat;
static uint64_t *rflags;

void read_rapl_init(void)
{
    init_msr();
    rapl_init(&rdat, &rflags);
    /* DRAM_PERF_STATUS not implemented yet in libmsr. */
    *rflags = *rflags & ~DRAM_PERF_STATUS;

    static struct rapl_limit rlim[4];
    get_pkg_rapl_limit(0, &(rlim[0]), NULL);
    get_pkg_rapl_limit(1, &(rlim[1]), NULL);
    get_dram_rapl_limit(0, &(rlim[2]));
    get_dram_rapl_limit(1, &(rlim[3]));
}

void read_rapl_energy_and_power(double *ret)
{
    static struct rapl_limit rlim[4];
    /* RAPL reads. */
    poll_rapl_data();
    get_pkg_rapl_limit(0, &(rlim[0]), NULL);
    get_pkg_rapl_limit(1, &(rlim[1]), NULL);
    get_dram_rapl_limit(0, &(rlim[2]));
    get_dram_rapl_limit(1, &(rlim[3]));

    ret[0] = rdat->pkg_delta_joules[0];
    ret[1] = rdat->pkg_delta_joules[1];
    ret[2] = rlim[0].watts * rdat->elapsed;
    ret[3] = rlim[1].watts * rdat->elapsed;
    ret[4] = rdat->pkg_delta_joules[0] / rdat->elapsed;
    ret[5] = rdat->pkg_delta_joules[1] / rdat->elapsed;
    ret[6] = rlim[0].watts;
    ret[7] = rlim[1].watts;
    ret[8] = rdat->dram_delta_joules[0];
    ret[9] = rdat->dram_delta_joules[1];
}

void set_rapl_power(double s0bound, double s1bound)
{
    static struct rapl_limit rlim[2];
    rlim[0].watts = s0bound;
    rlim[1].watts = s1bound;
    rlim[0].bits = 0;
    rlim[1].bits = 0;
    rlim[0].seconds = 1;
    rlim[1].seconds = 1;
    set_pkg_rapl_limit(0, &(rlim[0]), &(rlim[1]));
    set_pkg_rapl_limit(1, &(rlim[0]), &(rlim[1]));

    get_pkg_rapl_limit(0, &(rlim[0]), NULL);
    get_pkg_rapl_limit(1, &(rlim[1]), NULL);
}
