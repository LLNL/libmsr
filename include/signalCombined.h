/* signalCombined.h
 *
 * Copyright (c) 2011-2016, Lawrence Livermore National Security, LLC.
 * LLNL-CODE-645430
 *
 * Produced at Lawrence Livermore National Laboratory
 * Written by  Barry Rountree, rountree@llnl.gov
 *             Scott Walker,   walker91@llnl.gov
 *             Kathleen Shoga, shoga1@llnl.gov
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

#ifndef SIGNALCOMBINED_H_INCLUDE
#define SIGNALCOMBINED_H_INCLUDE

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>

#include "msr_core.h"
#include "msr_thermal.h"
#include "msr_rapl.h"
#include "msr_clocks.h"

#ifdef __cplusplus
extern "C" {
#endif

/// @brief Collect measurements from all MSRs on a regular interval.
///
/// This function seems to be obsolete.
///
/// @param [in] i Identifier of signal to handle.
void printData(int i);

#ifdef __cplusplus
}
#endif
#endif
