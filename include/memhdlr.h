/* 
 * Copyright (c) 2015, Lawrence Livermore National Security, LLC.  
 * Produced at the Lawrence Livermore National Laboratory  
 * Written by Scott Walker, walker91@llnl.gov.
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

#ifndef LIBMSR_MEM
#define LIBMSR_MEM
#include <stdlib.h>

// These functions are for libmsr use only. Use outside of libmsr may cause
// segfaults or disrupt libmsr functions.

//#define MEMHDLR_DEBUG
#define MEMERR_GENERIC (fprintf(stderr, "%s %s::%d ERROR: could not allocate memory\n", getenv("HOSTNAME"), __FILE__, __LINE__))

enum
{
    LIBMSR_FREE, LIBMSR_MALLOC, LIBMSR_CALLOC, LIBMSR_REALLOC, LIBMSR_FINALIZE
};

#ifdef __cplusplus
extern "C" {
#endif

void * libmsr_malloc(size_t size);
void * libmsr_calloc(size_t num, size_t size);
void * libmsr_realloc(void * addr, size_t size);
void * libmsr_free(void * addr);
void * memhdlr_finalize();
int memory_handler(void * address, void * oldaddr, int dealloc);

#ifdef __cplusplus
}
#endif

#endif
