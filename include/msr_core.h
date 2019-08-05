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

#ifndef MSR_CORE_H_INCLUDE
#define MSR_CORE_H_INCLUDE

#include <linux/types.h>
#include <stdint.h>
#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

#define NUM_DEVS (sockets * coresPerSocket * threadsPerCore)
#define X86_IOC_MSR_BATCH _IOWR('c', 0xA2, struct msr_batch_array)
#define MSR_BATCH_DIR "/dev/cpu/msr_batch"
#define FILENAME_SIZE 1024
//#define USE_NO_BATCH 1

/// @brief Enum encompassing type of data being read to/written from MSRs.
enum libmsr_data_type_e
{
    /// @brief Energy, time, and power measurements of various RAPL power
    /// domains.
    RAPL_DATA,
    /// @brief Units for energy, time, and power across all RAPL power domains.
    RAPL_UNIT,
    /// @brief Fixed-function counter measurements (i.e., instructions retired,
    /// reference clock cycles, CPU cycles).
    FIXED_COUNTERS_DATA,
    /// @brief Controls for fixed-function counters (i.e., instructions retired,
    /// reference clock cycles, CPU cycles).
    FIXED_COUNTERS_CTR_DATA,
    /// @brief General-purpose performance counter measurements.
    COUNTERS_DATA,
    /// @brief Controls for general-purpose performance counters and
    /// performance event select counter measurements.
    COUNTERS_CTRL,
    /// @brief Clock cycle measurements based on fixed frequency and actual
    /// frequency of the processor.
    CLOCKS_DATA,
    /// @brief Instantaneous operating frequency of the core or socket.
    PERF_DATA,
    /// @brief Thermal status of core.
    THERM_STAT,
    /// @brief Interrupts by thermal monitor when thermal sensor on a core is
    /// tripped.
    THERM_INTERR,
    /// @brief Thermal status of package.
    PKG_THERM_STAT,
    /// @brief Interrupts by thermal monitor when thermal sensor on the package
    /// is tripped.
    PKG_THERM_INTERR,
    /// @brief Current temperature of the package.
    TEMP_TARGET,
    /// @brief Software desired operating frequency of the core or socket.
    PERF_CTL,
    /// @brief Measured time spent in C-states by the package.
    PKG_CRES,
    /// @brief Measured time spent in C-states by the core.
    CORE_CRES,
    /// @brief Uncore performance event select counter measurements.
    UNCORE_EVTSEL,
    /// @brief Uncore general-performance counter measurements.
    UNCORE_COUNT,
    /// @brief User-defined batch MSR data.
    USR_BATCH0,
    /// @brief User-defined batch MSR data.
    USR_BATCH1,
    /// @brief User-defined batch MSR data.
    USR_BATCH2,
    /// @brief User-defined batch MSR data.
    USR_BATCH3,
    /// @brief User-defined batch MSR data.
    USR_BATCH4,
    /// @brief User-defined batch MSR data.
    USR_BATCH5,
    /// @brief User-defined batch MSR data.
    USR_BATCH6,
    /// @brief User-defined batch MSR data.
    USR_BATCH7,
    /// @brief User-defined batch MSR data.
    USR_BATCH8,
    /// @brief User-defined batch MSR data.
    USR_BATCH9,
    /// @brief User-defined batch MSR data.
    USR_BATCH10,
};

/// @brief Enum encompassing batch operations.
enum libmsr_batch_op_type_e
{
    /// @brief Load batch operation.
    BATCH_LOAD,
    /// @brief Write batch operation.
    BATCH_WRITE,
    /// @brief Read batch operation.
    BATCH_READ,
};

struct topo
{
    struct hwthread *thread_map;
    int discontinuous_mapping;
};

struct hwthread
{
    int apic_id;
    int thread_id;
    int core_id;
    int socket_id;
    int sibling;
};

// Depending on their scope, MSRs can be written to or read from at either the
// socket (aka package/cpu) or core level, and possibly the hardware thread
// level.
//
//  read/write_msr reads from core 0.
//  read/write_msr_all_cores_v uses a vector of values.
//  write_msr_all_cores writes all cores with a single value.
//  read/write_msr_single_core contains all of the low-level logic.
//	The rest of the functions are wrappers that call these two functions.

/// @brief Structure holding information for a single read/write operation to
/// an MSR.
struct msr_batch_op
{
    /// @brief CPU where rdmsr/wrmsr will be executed.
    __u16 cpu;
    /// @brief Identify if operation is rdmsr (non-zero) or wrmsr (0).
    __u16 isrdmsr;
    /// @brief Stores error code.
    __s32 err;
    /// @brief Address of MSR to perform operation.
    __u32 msr;
    /// @brief Stores input to or result from operation.
    __u64 msrdata;
    /// @brief Write mask applied to wrmsr.
    __u64 wmask;
};

/// @brief Structure holding multiple read/write operations to various MSRs.
struct msr_batch_array
{
    /// @brief Number of operations to execute.
    __u32 numops;
    /// @brief Array of length numops of operations to execute.
    struct msr_batch_op *ops;
};

/// @brief Retrieve the number of cores existing on the platform.
///
/// @return Number of cores.
uint64_t num_cores(void);

/// @brief Retrieve the number of sockets existing on the platform.
///
/// @return Number of sockets.
uint64_t num_sockets(void);

/// @brief Retrieve total number of logical processors existing on the platform.
///
/// @return Total number of logical processors.
uint64_t num_devs(void);

/// @brief Retrieve number of cores per socket existing on the platform.
///
/// @return Number of cores per socket.
uint64_t cores_per_socket(void);

/// @brief Allocate space for new batch operation.
///
/// @param [in] batchnum libmsr_data_type_e data type of batch operation.
///
/// @param [in] bsize Size of batch operation.
///
/// @return 0 if allocation was a success, else -1 if batch_storage() fails.
int allocate_batch(int batchnum,
                   size_t bsize);

/// @brief Deallocate memory for specific set of batch operations.
///
/// @param [in] batchnum libmsr_data_type_e data type of batch operation.
///
/// @return 0 if successful, else -1 if batch_storage() fails.
int free_batch(int batchnum);

/// @brief Create new batch operation.
///
/// @param [in] msr Address of MSR for which operation will take place.
///
/// @param [in] cpu CPU where batch operation will take place.
///
/// @param [in] dest Stores data resulting from rdmsr or necessary for wrmsr.
///
/// @param [in] batchnum libmsr_data_type_e data type of batch operation.
///
/// @return 0 if creation was a success, else -1 if batch_storage() fails or if
/// the number of batch operations exceeds the allocated size.
int create_batch_op(off_t msr,
                    uint64_t cpu,
                    uint64_t **dest,
                    const int batchnum);

/// @brief Detect platform configuration.
///
/// @param [out] coresPerSocket Number of cores per socket.
///
/// @param [out] threadsPerCore Number of threads per core.
///
/// @param [out] sockets Number of sockets.
///
/// @param [out] HTenabled Indicates if hyperthreading is enabled/disabled.
void core_config(uint64_t *coresPerSocket,
                 uint64_t *threadsPerCore,
                 uint64_t *sockets,
                 int *HTenabled);

/// @brief Validate specific socket exists in the platform configuration.
///
/// @param [in] socket Unique socket/package identifier.
///
/// @param [in] location Line number in source file where error occurred (use
///        standard predefined macro __LINE__).
///
/// @param [in] file Name of source file where error occurred (use standard
///        predefined macro __FILE__).
///
/// @return 0 if successful, else -1 if socket requested is greater than number
/// of sockets in the platform.
int sockets_assert(const unsigned *socket,
                   const int location,
                   const char *file);

/// @brief Validate specific thread exists in the platform configuration.
///
/// @param [in] thread Unique thread identifier.
///
/// @param [in] location Line number in source file where error occurred (use
///        standard predefined macro __LINE__).
///
/// @param [in] file Name of source file where error occurred (use standard
///        predefined macro __FILE__).
///
/// @return 0 if successful, else -1 if thread requested is greater than number
/// of threads per core in the platform.
int threads_assert(const unsigned *thread,
                   const int location,
                   const char *file);

/// @brief Validate specific core exists in the platform configuration.
///
/// @param [in] core Unique core identifier.
///
/// @param [in] location Line number in source file where error occurred (use
///        standard predefined macro __LINE__).
///
/// @param [in] file Name of source file where error occurred (use standard
///        predefined macro __FILE__).
///
/// @return 0 if successful, else -1 if core requested is greater than number
/// of cores per socket in the platform.
int cores_assert(const unsigned *core,
                 const int location,
                 const char *file);

/// @brief Check status of a file.
///
/// @param [in] filename File to check status of.
///
/// @param [in] kerneltype OS privilege level (ring 0-ring 3).
///
/// @param [in] dev_idx Unique logical processor index.
///
/// @return 0 if successful, else -1 if can't find any msr module with RW
/// permissions.
int stat_module(char *filename,
                int *kerneltype,
                int *dev_idx);

/// @brief Open the MSR module file descriptors exposed in the /dev filesystem.
///
/// @return 0 if initialization was a success, else -1 if could not stat file
/// descriptors or open any msr module.
int init_msr(void);

/// @brief Close the MSR module file descriptors exposed in the /dev
/// filesystem.
///
/// @return 0 if finalization was a success, else -1 if could not close file
/// descriptors.
int finalize_msr(void);

/// @brief Write a new value to an MSR based on the coordinates of a core or
/// thread.
///
/// A user can request to write to core 4 on socket 1, instead of having to map
/// this core to a continuous value based on the the number of cores on socket
/// 0. For a dual socket system with 8 cores, socket 0 would have cores 0-7,
/// core 0 on socket 1 would be mapped to index 8.
///
/// @param [in] socket Unique socket/package identifier.
///
/// @param [in] core Unique core identifier.
///
/// @param [in] thread Unique thread identifier.
///
/// @param [in] msr Address of register to write.
///
/// @param [in] val Value to write to MSR.
///
/// @return 0 if write_msr_by_idx() was a success, else -1 if the file
/// descriptor was NULL or if the number of bytes written was not the size of
/// uint64_t.
int write_msr_by_coord(unsigned socket,
                       unsigned core,
                       unsigned thread,
                       off_t msr,
                       uint64_t val);

/// @brief Read current value of an MSR based on the coordinates of a core or
/// thread.
///
/// A user can request to read from core 4 on socket 1, instead of having to map
/// this core to a continuous value based on the the number of cores on socket
/// 0. For a dual socket system with 8 cores, socket 0 would have cores 0-7,
/// core 0 on socket 1 would be mapped to index 8.
///
/// @param [in] socket Unique socket/package identifier.
///
/// @param [in] core Unique core identifier.
///
/// @param [in] thread Unique thread identifier.
///
/// @param [in] msr Address of register to read.
///
/// @param [out] val Value read from MSR.
///
/// @return 0 if read_msr_by_idx() was a success, else -1 if the file
/// descriptor was NULL or if the number of bytes read was not the size of
/// uint64_t.
int read_msr_by_coord(unsigned socket,
                      unsigned core,
                      unsigned thread,
                      off_t msr,
                      uint64_t *val);

/// @brief Perform batch read of multiple MSR based on the coordinates of a
/// core or thread.
///
/// @param [in] socket Unique socket/package identifier.
///
/// @param [in] core Unique core identifier.
///
/// @param [in] thread Unique thread identifier.
///
/// @param [in] msr Address of register to read.
///
/// @param [out] val Value read from MSR.
///
/// @param [in] batchnum libmsr_data_type_e data type of batch operation.
///
/// @return 0 if read_msr_by_idx() was a success, else -1 if the file
/// descriptor was NULL or if the number of bytes read was not the size of
/// uint64_t.
int read_msr_by_coord_batch(unsigned socket,
                            unsigned core,
                            unsigned thread,
                            off_t msr,
                            uint64_t **val,
                            int batchnum);

/// @brief Do batch read operation.
///
/// @param [in] batchnum libmsr_data_type_e data type of batch operation.
int read_batch(const int batchnum);

/// @brief Do batch write operation.
///
/// @param [in] batchnum libmsr_data_type_e data type of batch operation.
int write_batch(const int batchnum);

/// @brief Load batch operations for a socket.
///
/// @param [in] msr Address of register to load.
///
/// @param [in] val Pointer to batch storage array.
///
/// @param [in] batchnum libmsr_data_type_e data type of batch operation.
///
/// @return 0 if successful, else -1 if batch storage array is uninitialized.
int load_socket_batch(off_t msr,
                      uint64_t **val,
                      const int batchnum);

/// @brief Load batch operations for a core.
///
/// @param [in] msr Address of register to load.
///
/// @param [in] val Pointer to batch storage array.
///
/// @param [in] batchnum libmsr_data_type_e data type of batch operation.
///
/// @return 0 if successful, else -1 if batch storage array is uninitialized.
int load_core_batch(off_t msr,
                    uint64_t **val,
                    const int batchnum);

/// @brief Load batch operations for a thread.
///
/// @param [in] msr Address of register to load.
///
/// @param [in] val Pointer to batch storage array.
///
/// @param [in] batchnum libmsr_data_type_e data type of batch operation.
///
/// @return 0 if successful, else -1 if batch storage array is uninitialized.
int load_thread_batch(off_t msr,
                      uint64_t **val,
                      const int batchnum);

/// @brief Read current value of an MSR based on the index of a core or
/// thread.
///
/// A user can request to read from index 8, which is core 0 on socket 1 in a
/// dual socket system with 8 cores per socket.  This index is a continuous
/// value based on the the number of cores on socket 0.
///
/// @param [in] dev_idx Unique device identifier.
///
/// @param [in] msr Address of register to read.
///
/// @param [out] val Value read from MSR.
///
/// @return 0 if successful, else -1 if file descriptor was NULL or if the
/// number of bytes read was not the size of uint64_t.
int read_msr_by_idx(int dev_idx,
                    off_t msr,
                    uint64_t *val);

/// @brief Write new value to an MSR based on the index of a core or thread.
///
/// A user can request to read from index 8, which is core 0 on socket 1 in a
/// dual socket system with 8 cores per socket.  This index is a continuous
/// value based on the the number of cores on socket 0.
///
/// @param [in] dev_idx Unique device identifier.
///
/// @param [in] msr Address of register to read.
///
/// @param [out] val Value read from MSR.
///
/// @return 0 if successful, else -1 if file descriptor was NULL or if the
/// number of bytes read was not the size of uint64_t.
int write_msr_by_idx(int dev_idx,
                     off_t msr,
                     uint64_t val);

/// @brief Verify successful MSR write by following operation immediately with
/// a read.
//
/// @param [in] dev_idx Unique device identifier.
///
/// @param [in] msr Address of register to read.
///
/// @param [out] val Value read from MSR.
///
/// @return 0 if successful, else -1 if file descriptor was NULL, if the
/// number of bytes written was not the size of uint64_t, if the read function
/// failed, or if the value written does not match the read value.
int write_msr_by_idx_and_verify(int dev_idx,
                                off_t msr,
                                uint64_t val);

#ifdef __cplusplus
}
#endif
#endif
