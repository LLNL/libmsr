/* memhdlr.c
 *
 * Copyright (c) 2011-2015, Lawrence Livermore National Security, LLC. LLNL-CODE-645430
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

#include <stdlib.h>
#include <stdio.h>
#include "memhdlr.h"

// These functions are for internal libmsr use only. Use outside of libmsr may 
// cause segfaults or disruption of libmsr functions.

#ifdef __cplusplus
extern "C" {
#endif

void * libmsr_malloc(size_t size)
{
    void * result = malloc(size);
#ifdef MEMHDLR_DEBUG
    fprintf(stderr, "MEMHDLR: (malloc) allocated new array at %p\n", result);
#endif
    if (result == NULL)
    {
        MEMERR_GENERIC;
        exit(-1);
    }
    memory_handler(result, NULL, LIBMSR_MALLOC);
    return result;
}

void * libmsr_calloc(size_t num, size_t size)
{
    void * result = calloc(num, size);
#ifdef MEMHDLR_DEBUG
    fprintf(stderr, "MEMHDLR: (calloc) allocated new array at %p\n", result);
#endif
    if (result == NULL)
    {
        MEMERR_GENERIC;
        exit(-1);
    }
    memory_handler(result, NULL, LIBMSR_CALLOC);
    return result;
}

void * libmsr_realloc(void * addr, size_t size)
{
    void * result = realloc(addr, size);
#ifdef MEMHDLR_DEBUG
    fprintf(stderr, "MEMHDLR: (realloc) allocated new array at %p (from %p) with new size %lu\n", result, addr, size);
#endif
    if (result == NULL)
    {
        MEMERR_GENERIC;
        exit(-1);
    }
    memory_handler(result, addr, LIBMSR_REALLOC);
    return result;
}

void * libmsr_free(void * addr)
{
    memory_handler(NULL, addr, LIBMSR_FREE);
    return NULL;
}

void * memhdlr_finalize()
{
    memory_handler(NULL, NULL, LIBMSR_FINALIZE);
    return NULL;
}

// Keeps track of all dynamic arrays allocated in libmsr to make deallocation simple
int memory_handler(void * address, void * oldaddr, int type)
{
    static void ** arrays = NULL;
    static unsigned last = 0;
    static unsigned size = 3;
    int i;
    switch (type)
    {
        case LIBMSR_FINALIZE:
            for (i = 0; i < last; i++)
            {
                if (arrays[i])
                {
#ifdef MEMHDLR_DEBUG
                fprintf(stderr, "MEMHDLR: data at %p has been deallocated\n", arrays[i]);
#endif
                    free(arrays[i]);
                }
            }
            if (arrays)
            {
                free(arrays);
            }
            return 0;
        case LIBMSR_REALLOC:
#ifdef MEMHDLR_DEBUG
            fprintf(stderr, "MEMHDLR: recieved address %p, (realloc %p)\n", address, oldaddr);
#endif
            for (i = 0; i < last; i++)
            {
                if (arrays[i] == oldaddr)
                {
                    arrays[i] = address;
                }
            }
            return 0;
        case LIBMSR_FREE:
            for (i = 0; i < last; i++)
            {
                if (arrays[i] == oldaddr)
                {
                    free(oldaddr);
                    arrays[i] = NULL;
                }
            }
            return 0;
        case LIBMSR_CALLOC: case LIBMSR_MALLOC:
            if (arrays == NULL)
            {
                arrays = (void **) malloc(size * sizeof(void *));
                if (arrays == NULL)
                {
                    MEMERR_GENERIC;
                    exit(-1);
                }
            }
#ifdef MEMHDLR_DEBUG
            fprintf(stderr, "MEMHDLR: arrays tracker is at %p\n", arrays);
#endif
            if (last >= size)
            {
                void ** temp = arrays;
                size *= 2;
                temp = (void **) realloc(arrays, size * sizeof(void *));
                if (temp == NULL)
                {
                    MEMERR_GENERIC;
                    exit(-1);
                }
                arrays = temp;
            }
            if (address)
            {
                arrays[last] = address;
                last++;
            }
            return 0;
    }
    return 0;
}

#ifdef __cplusplus
}
#endif
