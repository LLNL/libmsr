/* x86_arch.c
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
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for
 * more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with libmsr.  If not, see <http://www.gnu.org/licenses/>.
 *
 * This material is based upon work supported by the U.S. Department of
 * Energy's Lawrence Livermore National Laboratory. Office of Science, under
 * Award number DE-AC52-07NA27344.
 *
 */

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

uint64_t detect_arch(void)
{
    uint64_t rax = 1;
    uint64_t rbx = 0;
    uint64_t rcx = 0;
    uint64_t rdx = 0;

    asm volatile(
        "cpuid"
        : "=a" (rax), "=b" (rbx), "=c" (rcx), "=d" (rdx)
        : "0" (rax), "2" (rcx));

    return ((rax >> 4) & 0xF) | ((rax >> 12) & 0xF0);
}

int main(int argc, char **argv)
{
    uint64_t arch = detect_arch();
    printf("%lX\n", arch);
    return EXIT_SUCCESS;
}
