/* rapl.h
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

/// @brief Initialize the library to read from the rapl MSRs.
void read_rapl_init(void);

/// @brief Read the current consumption and allocation.
///
/// Variable ret must be an array of length 8.
/// idx 0,1 report consumption in joules for sockets 0 and 1.
/// idx 2,3 report allocation in joules for sockets 0 and 1.
/// idx 4,5 report consumption in watts for sockets 0 and 1.
/// idx 6,7 report allocation in joules for sockets 0 and 1.
void read_rapl_energy_and_power(double *ret);

void set_rapl_power(double s0bound, double s1bound);

