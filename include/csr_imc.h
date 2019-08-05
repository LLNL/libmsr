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

#ifndef CSR_IMC_H_INCLUDE
#define CSR_IMC_H_INCLUDE

#include <linux/types.h>

#include "master.h"

/* Integrated Memory Controller (iMC) CSRs */

#define NUMCTRS 8

/// @brief Structure containing data of per-component performance counters.
struct pmonctrs_data
{
    uint64_t **ctr0;
    uint64_t **ctr1;
    uint64_t **ctr2;
    uint64_t **ctr3;
    uint64_t **ctr4;
    // config
    uint64_t **ctrcfg0;
    uint64_t **ctrcfg1;
    uint64_t **ctrcfg2;
    uint64_t **ctrcfg3;
    uint64_t **ctrcfg4;
};

/// @todo Document fixed_perfmon_data struct.
struct fixed_perfmon_data
{
    uint64_t **fctr;
    // config
    uint64_t **fctrcfg;
};

/// @todo Document pmonctr_global struct.
struct pmonctr_global
{
    uint64_t **unitctrl;
    uint64_t **unitstatus;
};

/// @brief Store the PMON counter data on the heap.
///
/// @return Pointer to PMON counter data.
struct pmonctrs_data *pmon_ctr_storage(void);

/// @brief the PMON global counter data on the heap.
///
/// @return Pointer to PMON global counter data.
struct pmonctr_global *pmonctr_global_storage(void);

/// @brief Initialize storage for PMON performance counter data.
///
/// @return -1 if PMON counters have been initialized, else
/// 10*NUMCTRS*num_sockets.
int init_pmon_ctrs(void);

/// @brief Initialize storage for PMON global performance counter data.
///
/// @return -1 if PMON global counters have been initialized, else
/// 2*NUMCTRS*num_sockets.
int init_pmonctr_global(void);

/// @brief Configure iMC performance counters.
///
/// @param [in] threshold Threshold used in counter comparison.
///
/// @param [in] ovf_en When set to 1 and the counter overflows, its overflow
///        bit is set in the local status register and global status register.
///
/// @param [in] edge_det When set to 1, increment counter when a 0 --> 1
///        transition (i.e., rising edge) is detected. When 0, the counter will
///        increment in each cycle that the event is asserted.
///
/// @param [in] umask Select subevents to be counted within the selected event.
///
/// @param [in] event Select event to be counted.
///
/// @param [in] counter Unique counter identifier.
///
/// @return 0 if set_pmon_config() and do_csr_batch_op() are successful, else
/// -1 if counter does not exist.
int pmon_config(uint32_t threshold,
                uint32_t ovf_en,
                uint32_t edge_det,
                uint32_t umask,
                uint8_t event,
                const unsigned counter);

/// @brief Set PMON controls.
///
/// @param [in] ovf_en When set to 1 and the counter overflows, its overflow
///        bit is set in the local status register and global status register.
///
/// @param [in] freeze_en Enable freezing of all uncore performance counters.
///
/// @param [in] freeze Freeze all uncore performance monitors.
///
/// @param [in] reset When set to 1, the corresponding counter will be cleared
///        to 0.
///
/// @param [in] reset_cf Reset to see a new freeze.
///
/// @return 0 if do_csr_batch_op() was successful, else -1.
int set_pmon_unit_ctrl(uint32_t ovf_en,
                       uint16_t freeze_en,
                       uint16_t freeze,
                       uint16_t reset,
                       uint8_t reset_cf);

/// @brief iMC memory bandwidth performance counter.
///
/// @param [in] counter Unique counter identifier.
///
/// @param [in] type csr_data_type_e data type of event.
///
/// @return 0 if pmon_config() was successful, else -1 if counter does not
/// exist or if specified bandwidth measurement is invalid.
int mem_bw_on_ctr(const unsigned counter,
                  const int type);

/// @todo Document mem_pct_rw_on_ctr().
///
/// @return 0 if pmon_config() was successful, else -1 if counter does not exist.
int mem_pct_rw_on_ctr(const unsigned rcounter,
                      const unsigned wcounter);

/// @todo Document mem_page_empty_on_ctr().
///
/// @return 0 if pmon_config() was successful, else -1 if number of counters is
/// larger than number of iMC performance counters.
int mem_page_empty_on_ctr(const unsigned act_count,
                          const unsigned pre_count,
                          const unsigned cas_count);

/// @todo Document mem_page_miss_on_ctr().
///
/// @return 0 if pmon_config() was successful, else -1 if counter does not
/// exist.
int mem_page_miss_on_ctr(const unsigned pre_count,
                         const unsigned cas_count);

/// @brief Read iMC counters.
///
/// @param [in] counter Unique counter identifier.
///
/// @return 0 if do_csr_batch_op() was successful, else -1 if counter does not
/// exist.
int read_imc_counter_batch(const unsigned counter);

/// @brief Print memory bandwidth.
///
/// @param [in] counter Unique counter identifier.
///
/// @param [in] writedest File stream where output will be written to.
///
/// @return 0 if successful.
int print_mem_bw_from_ctr(const unsigned counter,
                          FILE *writedest);

/// @brief Print percentage of read/write requests.
///
/// @param [in] rcounter Unique counter identifier for read requests.
///
/// @param [in] wcounter Unique counter identifier for write requests.
///
/// @param [in] type Indicates data is reporting read or write requests.
///
/// @param [in] writedest File stream where output will be written to.
///
/// @return 0 if successful.
int print_mem_pct_rw_from_ctr(const unsigned rcounter,
                              const unsigned wcounter,
                              int type,
                              FILE *writedest);

/// @brief Print percentage of read/write requests that caused a page empty.
///
/// @todo Document print_mem_page_empty_from_ctr() parameters.
///
/// @return 0 if successful.
int print_mem_page_empty_from_ctr(const unsigned act,
                                  const unsigned pre,
                                  const unsigned cas,
                                  FILE *writedest);

/// @brief Print percentage of read/write requests that caused a page miss.
///
/// @todo Document print_mem_page_miss_from_ctr() parameters.
///
/// @return 0 if successful.
int print_mem_page_miss_from_ctr(const unsigned pre,
                                 const unsigned cas,
                                 FILE *writedest);

/// @brief Print contents of iMC performance monitoring counters.
///
/// @return 0 if successful, else -1 if init_pmon_ctrs() fails.
int print_pmon_ctrs(void);

#endif
