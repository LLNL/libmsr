/* rapl2.c
 *
 * Copyright (c) 2011-2016, Lawrence Livermore National Security, LLC.
 * LLNL-CODE-645430
 *
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
 * Software Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * libmsr is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
 * details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with libmsr. If not, see <http://www.gnu.org/licenses/>.
 *
 * This material is based upon work supported by the U.S. Department of
 * Energy's Lawrence Livermore National Laboratory. Office of Science, under
 * Award number DE-AC52-07NA27344.
 *
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

    static struct rapl_limit rlim[2];
    get_pkg_rapl_limit(0, &(rlim[0]), NULL);
    get_pkg_rapl_limit(1, &(rlim[1]), NULL);
}

void read_rapl_energy_and_power(double *ret)
{
    static struct rapl_limit rlim[2];
    /* RAPL reads. */
    poll_rapl_data();
    get_pkg_rapl_limit(0, &(rlim[0]), NULL);
    get_pkg_rapl_limit(1, &(rlim[1]), NULL);

    ret[0] = rdat->pkg_delta_joules[0];
    ret[1] = rdat->pkg_delta_joules[1];
    ret[2] = rlim[0].watts * rdat->elapsed;
    ret[3] = rlim[1].watts * rdat->elapsed;
    ret[4] = rdat->pkg_delta_joules[0] / rdat->elapsed;
    ret[5] = rdat->pkg_delta_joules[1] / rdat->elapsed;
    ret[6] = rlim[0].watts;
    ret[7] = rlim[1].watts;
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
