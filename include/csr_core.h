/*
 * Copyright (c) 2013-2017, Lawrence Livermore National Security, LLC.
 *
 * Produced at the Lawrence Livermore National Laboratory. Written by:
 *     Barry Rountree <rountree@llnl.gov>,
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

#ifndef CSR_CORE_H_INCLUDE
#define CSR_CORE_H_INCLUDE

#include <errno.h>
#include <fcntl.h>
#include <linux/ioctl.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

#define CSRSAFE_8086_BATCH _IOWR('a', 0x05, struct csr_batch_array)
#define CSR_FILENAME_SIZE 128
#define CSR_MODULE "/dev/cpu/csr_safe"

/// @brief Enum encompassing type of data being read to/written from uncore
/// registers.
enum csr_data_type_e
{
    /// @brief Integrated memory controller counter measurements.
    CSR_IMC_CTRS,
    /// @brief Integrated memory controller performance event measurements.
    CSR_IMC_EVTS,
    /// @brief UBox PMON global control data for integrated memory controller.
    CSR_IMC_PMONUNITCTRL,
    /// @brief UBox PMON global status data for integrated memory controller.
    CSR_IMC_PMONUNITSTAT,
    /* Currently unused */
    //CSR_IMC_MEMCTRA,
    //CSR_IMC_MEMCTRR,
    //CSR_IMC_MEMCTRW,
    //CSR_IMC_IMCCTR,
    //CSR_QPI_CTRS,
    //CSR_QPI_EVTS
};

/// @brief Structure holding information for a single read/write operation to
/// an uncore register.
struct csr_batch_op
{
    /// @brief Bus identifier where read/write of uncore register will be
    /// executed.
    uint8_t bus;
    /// @brief Device identifier where read/write of uncore register will be
    /// executed.
    uint8_t device;
    /// @brief Function identifier where read/write of uncore register will be
    /// executed.
    uint8_t function;
    /// @brief Socket identifier where read/write of uncore register will be
    /// executed.
    uint8_t socket;
    /// @brief Address of uncore register to perform operation.
    uint16_t offset;
    /// @brief Stores error code.
    int err;
    /// @brief Identify if operation is read (non-zero) or write (0).
    uint8_t isread;
    /// @brief Stores input to or result from operation.
    uint64_t csrdata;
    /// @brief Write mask applied to uncore register.
    uint64_t wmask;
    /// @brief Number of batch operations.
    uint8_t size;
};

/// @brief Structure holding multiple read/write operations to various MSRs.
struct csr_batch_array
{
    /// @brief Number of operations to execute.
    uint32_t numops;
    /// @brief Array of length numops of operations to execute.
    struct csr_batch_op *ops;
};

/// @brief Open the module file descriptors exposed in the /dev filesystem.
///
/// @return 0 if initialization was a success, else -1 if could not stat file
/// descriptors or open any csr module.
int init_csr(void);

/// @brief Close the module file descriptors exposed in the /dev filesystem.
///
/// @return 0 if finalization was a success, else -1 if could not close file
/// descriptors.
int finalize_csr(void);

/// @brief Allocate space for uncore batch arrays.
///
/// @param [out] batchsel Storage for uncore batch operations.
///
/// @param [in] batchnum csr_data_type_e data type of batch operation.
///
/// @param [out] opssize Size of specific set of batch operations.
///
/// @return 0 if successful, else NULL pointer as a result of libmsr_calloc()
/// or libmsr_realloc().
int csr_batch_storage(struct csr_batch_array **batchsel,
                      const int batchnum,
                      unsigned **opssize);

/// @brief Allocate space for new uncore batch operation.
///
/// @param [in] batchnum csr_data_type_e data type of batch operation.
///
/// @param [in] bsize Size of batch operation.
///
/// @return 0 if allocation was a success, else -1 if csr_batch_storage() fails.
int allocate_csr_batch(const int batchnum,
                       size_t bsize);

/// @brief Deallocate memory for specific set of uncore batch operations.
///
/// @param [in] batchnum csr_data_type_e data type of batch operation.
///
/// @return 0 if successful, else -1 if csr_batch_storage() fails.
int free_csr_batch(const int batchnum);

/// @brief Create new uncore batch operation.
///
/// @param [in] csr Address of uncore register for which operation will take
///             place.
///
/// @param [in] bus Bus where batch operation will take place.
///
/// @param [in] device Device where batch operation will take place.
///
/// @param [in] function Function where batch operation will take place.
///
/// @param [in] socket Socket where batch operation will take place.
///
/// @param [in] isread Indicates read or write to uncore register.
///
/// @param [in] opsize Size of operation.
///
/// @param [in] dest Stores data resulting from read or necessary for write to
///             uncore register.
///
/// @param [in] batchnum csr_data_type_e data type of batch operation.
///
/// @return 0 if creation was a success, else -1 if csr_batch_storage() fails
/// or if the number of batch operations exceeds the allocated size.
int create_csr_batch_op(off_t csr,
                        uint8_t bus,
                        uint8_t device,
                        uint8_t function,
                        uint8_t socket,
                        uint8_t isread,
                        size_t opsize,
                        uint64_t **dest,
                        const int batchnum);

/// @brief Execute read/write batch operation on a specific set of batch
/// uncore registers.
///
/// @param [in] batchnum csr_data_type_e data type of batch operation.
///
/// @return 0 if successful, else -1 if csr_batch_storage() fails or if batch
/// allocation is for 0 or less operations.
int do_csr_batch_op(const int batchnum);

#endif
