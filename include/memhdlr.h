/* memhdlr.h
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

#ifndef MEMHDLR_H_INCLUDE
#define MEMHDLR_H_INCLUDE

#include <stdlib.h>

// These functions are for libmsr use only. Use outside of libmsr may cause
// segfaults or disrupt libmsr functions.
// The primary purpose of these functions is to simplify memory management and
// debugging (debugging much easier), but there are plans to add more
// functionality in the future.

#ifdef __cplusplus
extern "C" {
#endif

/// @brief Enum encompassing types of libmsr memory management.
enum libmsr_mem_mgmt_e {
    LIBMSR_FREE,
    LIBMSR_MALLOC,
    LIBMSR_CALLOC,
    LIBMSR_REALLOC,
    LIBMSR_FINALIZE
};

/// @brief Allocate size bytes with malloc().
///
/// @param [in] size Number of bytes.
///
/// @return Pointer to dynamic array.
void *libmsr_malloc(size_t size);

/// @brief Allocate memory for an array of num elements of size bytes each with
/// calloc().
///
/// @param [in] size Number of bytes.
///
/// @param [in] num Number of elements.
///
/// @return Pointer to dynamic array.
void *libmsr_calloc(size_t num,
                    size_t size);

/// @brief Change allocation of existing memory to size bytes with realloc().
///
/// @param [out] addr Pointer to dynamic array.
///
/// @param [in] size Number of bytes.
///
/// @return Pointer to dynamic array.
void *libmsr_realloc(void *addr,
                     size_t size);

/// @brief Free single dynamic memory allocation.
///
/// @param [in] addr Pointer to dynamic array.
///
/// @return Null pointer.
void *libmsr_free(void *addr);

/// @brief Deallocate any remaining memory allocations and deallocate tracking
/// structure.
///
/// @return Null pointer.
void *memhdlr_finalize(void);

/// @brief Track all dynamic arrays allocated in libmsr to make deallocation
/// simple.
///
/// @param [out] address Pointer to dynamic array.
///
/// @param [out] oldaddr Pointer to dynamic array.
///
/// @param [in] type libmsr_mem_mgmt_e memory management identifier.
///
/// @return 0 if successful, -1 if malloc() or realloc() fails.
int memory_handler(void *address,
                   void *oldaddr,
                   int type);

#ifdef __cplusplus
}
#endif
#endif
