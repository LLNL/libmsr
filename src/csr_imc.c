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

#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "csr_core.h"
#include "memhdlr.h"
#include "msr_core.h"
#include "csr_imc.h"
#include "cpuid.h"
#include "libmsr_debug.h"
#include "libmsr_error.h"

/// @brief Load batch operations for the integrated memory controller (iMC).
///
/// @param [in] offt Address of uncore register to load.
///
/// @param [in] loc Pointer to csr batch storage array.
///
/// @param [in] isread Indicates read or write to uncore register.
///
/// @param [in] size Number of batch operations.
///
/// @param [in] batchno csr_data_type_e data type of batch operation.
///
/// @return Number of csr batch operations created.
static int load_imc_batch_for_each(const size_t offt, uint64_t **loc, const int isread, const size_t size, const unsigned batchno)
{
    int idx = 0;

    if (loc)
    {
        create_csr_batch_op(offt, 1, IMC0_DEV, IMC_CH0_FUNC, 0, isread, size, &loc[idx], batchno);
        create_csr_batch_op(offt, 1, IMC0_DEV, IMC_CH1_FUNC, 0, isread, size, &loc[++idx], batchno);
        create_csr_batch_op(offt, 1, IMC0_DEV, IMC_CH2_FUNC, 0, isread, size, &loc[++idx], batchno);
        create_csr_batch_op(offt, 1, IMC0_DEV, IMC_CH3_FUNC, 0, isread, size, &loc[++idx], batchno);

        create_csr_batch_op(offt, 1, IMC1_DEV, IMC_CH0_FUNC, 0, isread, size, &loc[++idx], batchno);
        create_csr_batch_op(offt, 1, IMC1_DEV, IMC_CH1_FUNC, 0, isread, size, &loc[++idx], batchno);
        create_csr_batch_op(offt, 1, IMC1_DEV, IMC_CH2_FUNC, 0, isread, size, &loc[++idx], batchno);
        create_csr_batch_op(offt, 1, IMC1_DEV, IMC_CH3_FUNC, 0, isread, size, &loc[++idx], batchno);

        if (num_sockets() > 1)
        {
            create_csr_batch_op(offt, 1, IMC0_DEV, IMC_CH0_FUNC, 1, isread, size, &loc[++idx], batchno);
            create_csr_batch_op(offt, 1, IMC0_DEV, IMC_CH1_FUNC, 1, isread, size, &loc[++idx], batchno);
            create_csr_batch_op(offt, 1, IMC0_DEV, IMC_CH2_FUNC, 1, isread, size, &loc[++idx], batchno);
            create_csr_batch_op(offt, 1, IMC0_DEV, IMC_CH3_FUNC, 1, isread, size, &loc[++idx], batchno);

            create_csr_batch_op(offt, 1, IMC1_DEV, IMC_CH0_FUNC, 1, isread, size, &loc[++idx], batchno);
            create_csr_batch_op(offt, 1, IMC1_DEV, IMC_CH1_FUNC, 1, isread, size, &loc[++idx], batchno);
            create_csr_batch_op(offt, 1, IMC1_DEV, IMC_CH2_FUNC, 1, isread, size, &loc[++idx], batchno);
            create_csr_batch_op(offt, 1, IMC1_DEV, IMC_CH3_FUNC, 1, isread, size, &loc[++idx], batchno);
        }
    }
    else
    {
        libmsr_error_handler("load_imc_batch_for_each(): Unable to create iMC batch -- Loc was null", LIBMSR_ERROR_MSR_BATCH, getenv("HOSTNAME"), __FILE__, __LINE__);
    }
    return idx;
}

struct pmonctrs_data *pmon_ctr_storage(void)
{
    static struct pmonctrs_data pcd;
    static int init = 0;

    if (!init)
    {
        int allocated = NUMCTRS * num_sockets();
        pcd.ctr0 = (uint64_t **) libmsr_calloc(allocated, sizeof(uint64_t *));
        pcd.ctr1 = (uint64_t **) libmsr_calloc(allocated, sizeof(uint64_t *));
        pcd.ctr2 = (uint64_t **) libmsr_calloc(allocated, sizeof(uint64_t *));
        pcd.ctr3 = (uint64_t **) libmsr_calloc(allocated, sizeof(uint64_t *));
        pcd.ctr4 = (uint64_t **) libmsr_calloc(allocated, sizeof(uint64_t *));
        // config
        pcd.ctrcfg0 = (uint64_t **) libmsr_calloc(allocated, sizeof(uint64_t *));
        pcd.ctrcfg1 = (uint64_t **) libmsr_calloc(allocated, sizeof(uint64_t *));
        pcd.ctrcfg2 = (uint64_t **) libmsr_calloc(allocated, sizeof(uint64_t *));
        pcd.ctrcfg3 = (uint64_t **) libmsr_calloc(allocated, sizeof(uint64_t *));
        pcd.ctrcfg4 = (uint64_t **) libmsr_calloc(allocated, sizeof(uint64_t *));
        init = 1;
    }
    return &pcd;
}

struct pmonctr_global *pmonctr_global_storage(void)
{
    static struct pmonctr_global pgd;
    static int init = 0;

    if (!init)
    {
        int allocated = NUMCTRS * num_sockets();
        pgd.unitctrl = (uint64_t **) libmsr_calloc(allocated, sizeof(uint64_t *));
        pgd.unitstatus = (uint64_t **) libmsr_calloc(allocated, sizeof(uint64_t *));
        init = 1;
    }
    return &pgd;
};

/// @todo Combine into 1 counter batch and 1 event batch.
int init_pmon_ctrs(void)
{
    static int init = 0;
    static int allocated = 0;
    static struct pmonctrs_data *pmonctrs;

    if (!init)
    {
        init = 1;
        allocated = NUMCTRS * num_sockets();
        pmonctrs = pmon_ctr_storage();

        allocate_csr_batch(CSR_IMC_CTRS, allocated * 4);
        allocate_csr_batch(CSR_IMC_EVTS, allocated * 4);

        load_imc_batch_for_each(CSR_PMONCTR0, pmonctrs->ctr0, 1, 8, CSR_IMC_CTRS);
        load_imc_batch_for_each(CSR_PMONCTR1, pmonctrs->ctr1, 1, 8, CSR_IMC_CTRS);
        load_imc_batch_for_each(CSR_PMONCTR2, pmonctrs->ctr2, 1, 8, CSR_IMC_CTRS);
        load_imc_batch_for_each(CSR_PMONCTR3, pmonctrs->ctr3, 1, 8, CSR_IMC_CTRS);

        load_imc_batch_for_each(CSR_PMONCTRCFG0, pmonctrs->ctrcfg0, 0, 4, CSR_IMC_EVTS);
        load_imc_batch_for_each(CSR_PMONCTRCFG1, pmonctrs->ctrcfg1, 0, 4, CSR_IMC_EVTS);
        load_imc_batch_for_each(CSR_PMONCTRCFG2, pmonctrs->ctrcfg2, 0, 4, CSR_IMC_EVTS);
        load_imc_batch_for_each(CSR_PMONCTRCFG3, pmonctrs->ctrcfg3, 0, 4, CSR_IMC_EVTS);

        return 10 * allocated;
    }
    return -1;
}

int init_pmonctr_global(void)
{
    static int init = 0;
    static int allocated = 0;
    static struct pmonctr_global *pgd;

    if (!init)
    {
        init = 1;
        allocated = NUMCTRS * num_sockets();
        pgd = pmonctr_global_storage();
        allocate_csr_batch(CSR_IMC_PMONUNITCTRL, allocated);
        allocate_csr_batch(CSR_IMC_PMONUNITSTAT, allocated);

        load_imc_batch_for_each(CSR_PMONUNITCTRL, pgd->unitctrl, 0, 3, CSR_IMC_PMONUNITCTRL);
        load_imc_batch_for_each(CSR_PMONUNITSTAT, pgd->unitstatus, 0, 1, CSR_IMC_PMONUNITSTAT);

        return 2 * allocated;
    }
    return -1;
}

/// @brief Set iMC performance monitoring configuration.
///
/// @param [in] setting Raw bits to set for configuration.
///
/// @param [out] cfg Pointer to raw configuration value.
///
/// @return 0 if successful, else -1 if iMC PMON config is uninitialized.
static int set_pmon_config(const uint32_t setting, uint64_t **cfg)
{
    int i;

    if (!*cfg)
    {
        libmsr_error_handler("set_pmon_config(): iMC PMON config is not initialized", LIBMSR_ERROR_CSR_INIT, getenv("HOSTNAME"), __FILE__, __LINE__);
        return -1;
    }
    for (i = 0; i < NUMCTRS * num_sockets(); i++)
    {
        *cfg[i] = setting;
    }
    return 0;
}

int pmon_config(uint32_t threshold, uint32_t ovf_en, uint32_t edge_det, uint32_t umask, uint8_t event, const unsigned counter)
{
    struct pmonctrs_data *pcd = pmon_ctr_storage();
    int ret0 = 0;
    int ret1 = 0;

    ovf_en &= 0x1;
    edge_det &= 0x1;
    uint32_t setting = 0x0 | (threshold << 24) | (0x1 << 22) | (ovf_en << 20) | (edge_det << 18) | (0x1 << 17) | (0x1 << 16) | (umask << 8) | event;
#ifdef CSRDEBUG
    fprintf(stderr, "CSRDEBUG: counter %u setting %x\n", counter, setting);
#endif
    switch (counter)
    {
        case 0:
            ret0 = set_pmon_config(setting, pcd->ctrcfg0);
            ret1 = do_csr_batch_op(CSR_IMC_EVTS);
            break;
        case 1:
            ret0 = set_pmon_config(setting, pcd->ctrcfg1);
            ret1 = do_csr_batch_op(CSR_IMC_EVTS);
            break;
        case 2:
            ret0 = set_pmon_config(setting, pcd->ctrcfg2);
            ret1 = do_csr_batch_op(CSR_IMC_EVTS);
            break;
        case 3:
            ret0 = set_pmon_config(setting, pcd->ctrcfg3);
            ret1 = do_csr_batch_op(CSR_IMC_EVTS);
            break;
        case 4:
            ret0 = set_pmon_config(setting, pcd->ctrcfg4);
            ret1 = do_csr_batch_op(CSR_IMC_EVTS);
            break;
        default:
            libmsr_error_handler("pmon_config(): iMC perfmon counter does not exist", LIBMSR_ERROR_CSR_COUNTERS, getenv("HOSTNAME"), __FILE__, __LINE__);
            return -1;
    }
    return ret0 | ret1;
}

int set_pmon_unit_ctrl(uint32_t ovf_en, uint16_t freeze_en, uint16_t freeze, uint16_t reset, uint8_t reset_cfg)
{
    struct pmonctr_global *pgd = pmonctr_global_storage();
    uint32_t setting = 0x0 | (ovf_en << 17) | (freeze_en << 16) | (freeze << 8) | (reset << 1) | reset_cfg;
    int i;

    for (i = 0; i < NUMCTRS * num_sockets(); i++)
    {
        *pgd->unitctrl[i] = setting;
    }
    return do_csr_batch_op(CSR_IMC_PMONUNITCTRL);
}

int mem_bw_on_ctr(const unsigned counter, const int type)
{
    int res = 0;

    if (counter > 4)
    {
        libmsr_error_handler("mem_bw_on_ctr(): iMC pmon counter does not exist", LIBMSR_ERROR_CSR_COUNTERS, getenv("HOSTNAME"), __FILE__, __LINE__);
        return -1;
    }
    switch (type)
    {
        case 0:
            res = pmon_config(0x0, 0x0, 0x0, UMASK_CAS_RD, EVT_CAS_COUNT, counter);
            break;
        case 1:
            res = pmon_config(0x0, 0x0, 0x0, UMASK_CAS_WR, EVT_CAS_COUNT, counter);
            break;
        case 2:
            res = pmon_config(0x0, 0x0, 0x0, UMASK_CAS_ALL, EVT_CAS_COUNT, counter);
            break;
        default:
            libmsr_error_handler("mem_bw_on_ctr(): invalid bandwidth measurement specifier", LIBMSR_ERROR_RUNTIME, getenv("HOSTNAME"), __FILE__, __LINE__);
            return -1;
    }
    return res;
}

int mem_pct_rw_on_ctr(const unsigned rcounter, const unsigned wcounter)
{
    int res = 0;

    if (rcounter > 4 || wcounter > 4)
    {
        libmsr_error_handler("mem_pct_rw_on_ctr(): iMC pmon counter does not exist", LIBMSR_ERROR_CSR_COUNTERS, getenv("HOSTNAME"), __FILE__, __LINE__);
        return -1;
    }
    if (rcounter == wcounter)
    {
        libmsr_error_handler("mem_pct_rw_on_ctr(): you can't count different events on the same register", LIBMSR_ERROR_RUNTIME, getenv("HOSTNAME"), __FILE__, __LINE__);
    }
    res |= pmon_config(0x0, 0x0, 0x0, 0x0, EVT_RPQ_INSERTS, rcounter);
    res |= pmon_config(0x0, 0x0, 0x0, 0x0, EVT_WPQ_INSERTS, wcounter);
    return res;
}

int mem_page_empty_on_ctr(const unsigned act_count, const unsigned pre_count, const unsigned cas_count)
{
    int res = 0;

    if (act_count > 4 || pre_count > 4 || cas_count > 4)
    {
        libmsr_error_handler("mem_page_empty_on_ctr(): there are only 4 iMC performance counters", LIBMSR_ERROR_CSR_COUNTERS, getenv("HOSTNAME"), __FILE__, __LINE__);
        return -1;
    }
    if (act_count == pre_count || cas_count == pre_count || cas_count == act_count)
    {
        libmsr_error_handler("mem_page_emptys_on_ctr(): you can't count different events on the same register", LIBMSR_ERROR_RUNTIME, getenv("HOSTNAME"), __FILE__, __LINE__);
    }
    res |= pmon_config(0x0, 0x0, 0x0, UMASK_CAS_ALL, EVT_CAS_COUNT, cas_count);
    res |= pmon_config(0x0, 0x0, 0x0, 0xFF, EVT_ACT_COUNT, act_count);
    res |= pmon_config(0x0, 0x0, 0x0, UMASK_PRE_PAGE_MISS, EVT_PRE_COUNT, pre_count);
    return res;
}

int mem_page_miss_on_ctr(const unsigned pre_count, const unsigned cas_count)
{
    int res = 0;

    if (pre_count > 4 || pre_count > 4)
    {
        libmsr_error_handler("mem_page_miss_on_ctr(): iMC PMON counter does not exist", LIBMSR_ERROR_CSR_COUNTERS, getenv("HOSTNAME"), __FILE__, __LINE__);
        return -1;
    }
    if (pre_count == cas_count)
    {
        libmsr_error_handler("mem_page_miss_on_ctr(): you can't count different events on the same register", LIBMSR_ERROR_RUNTIME, getenv("HOSTNAME"), __FILE__, __LINE__);
    }
    res |= pmon_config(0x0, 0x0, 0x0, UMASK_PRE_PAGE_MISS, EVT_PRE_COUNT, pre_count);
    res |= pmon_config(0x0, 0x0, 0x0, UMASK_CAS_ALL, EVT_CAS_COUNT, cas_count);
    return res;
}

int read_imc_counter_batch(const unsigned counter)
{
    int res = 0;

    switch (counter)
    {
        case 0:
            res = do_csr_batch_op(CSR_IMC_CTRS);
            break;
        case 1:
            res = do_csr_batch_op(CSR_IMC_CTRS);
            break;
        case 2:
            res = do_csr_batch_op(CSR_IMC_CTRS);
            break;
        case 3:
            res = do_csr_batch_op(CSR_IMC_CTRS);
            break;
        case 4:
            res = do_csr_batch_op(CSR_IMC_CTRS);
            break;
        default:
            libmsr_error_handler("read_imc_counter_batch(): iMC PMON counter does not exist", LIBMSR_ERROR_CSR_COUNTERS, getenv("HOSTNAME"), __FILE__, __LINE__);
            return -1;
            break;
    }
    return res;
}

int print_mem_bw_from_ctr(const unsigned counter, FILE *writedest)
{
    struct pmonctrs_data *pcd = pmon_ctr_storage();
    uint64_t **ctr = NULL;
    uint8_t devctr = 0;
    uint8_t func = 0;
    uint8_t sockctr = 0;
    int i;
    const uint8_t funcnums[4] = {IMC_CH0_FUNC, IMC_CH1_FUNC, IMC_CH2_FUNC, IMC_CH3_FUNC};
    const uint8_t devnums[2] = {IMC0_DEV, IMC1_DEV};

    read_imc_counter_batch(counter);
    switch (counter)
    {
        case 0:
            ctr = pcd->ctr0;
            break;
        case 1:
            ctr = pcd->ctr1;
            break;
        case 2:
            ctr = pcd->ctr2;
            break;
        case 3:
            ctr = pcd->ctr3;
            break;
        case 4:
            ctr = pcd->ctr4;
            break;
    }

    fprintf(writedest, "Memory Bandwidth\n");
    for (i = 0; i < NUMCTRS * num_sockets(); i++)
    {
        if (i > 0 && i%4 == 0)
        {
            devctr++;
        }
        devctr %= 2;
        func %= 4;
        if (i > 0 && i%8 == 0)
        {
            sockctr++;
        }
        fprintf(writedest, "dev %d func %d sock %d: ", devnums[devctr], funcnums[func], sockctr);
        fprintf(writedest, "%lu bytes\n", *ctr[i] * 64LU);
        func++;
    }
    return 0;
}

int print_mem_pct_rw_from_ctr(const unsigned rcounter, const unsigned wcounter, int type, FILE *writedest)
{
    struct pmonctrs_data *pcd = pmon_ctr_storage();
    uint64_t **rctr = NULL;
    uint64_t **wctr = NULL;
    const uint8_t funcnums[4] = {IMC_CH0_FUNC, IMC_CH1_FUNC, IMC_CH2_FUNC, IMC_CH3_FUNC};
    const uint8_t devnums[2] = {IMC0_DEV, IMC1_DEV};
    uint8_t devctr = 0;
    uint8_t func = 0;
    uint8_t sockctr = 0;
    int i;

    read_imc_counter_batch(rcounter);
    read_imc_counter_batch(wcounter);
    switch (rcounter)
    {
        case 0:
            rctr = pcd->ctr0;
            break;
        case 1:
            rctr = pcd->ctr1;
            break;
        case 2:
            rctr = pcd->ctr2;
            break;
        case 3:
            rctr = pcd->ctr3;
            break;
        case 4:
            rctr = pcd->ctr4;
            break;
    }
    switch (wcounter)
    {
        case 0:
            wctr = pcd->ctr0;
            break;
        case 1:
            wctr = pcd->ctr1;
            break;
        case 2:
            wctr = pcd->ctr2;
            break;
        case 3:
            wctr = pcd->ctr3;
            break;
        case 4:
            wctr = pcd->ctr4;
            break;
    }

    fprintf(writedest, "Percent %s Requests\n", (type ? "read\0" : "write\0"));
    for (i = 0; i < NUMCTRS * num_sockets(); i++)
    {
        if (i > 0 && i%4 == 0)
        {
            devctr++;
        }
        devctr %= 2;
        func %= 4;
        if (i > 0 && i%8 == 0)
        {
            sockctr++;
        }
        fprintf(writedest, "dev %d func %d sock %d: ", devnums[devctr], funcnums[func], sockctr);
        if (type)
        {
            fprintf(writedest, "%lf\n", (double)*rctr[i] / ((*rctr[i] + *wctr[i]) ? (*rctr[i] + *wctr[i]) : 1));
        }
        else
        {
            fprintf(writedest, "%lf\n", (double)*wctr[i] / ((*rctr[i] + *wctr[i]) ? (*rctr[i] + *wctr[i]) : 1));
        }
        func++;
    }
    return 0;
}

int print_mem_page_empty_from_ctr(const unsigned act, const unsigned pre, const unsigned cas, FILE *writedest)
{
    struct pmonctrs_data *pcd = pmon_ctr_storage();
    uint64_t **actr = NULL;
    uint64_t **pctr = NULL;
    uint64_t **cctr = NULL;
    const uint8_t funcnums[4] = {IMC_CH0_FUNC, IMC_CH1_FUNC, IMC_CH2_FUNC, IMC_CH3_FUNC};
    const uint8_t devnums[2] = {IMC0_DEV, IMC1_DEV};
    uint8_t devctr = 0;
    uint8_t func = 0;
    uint8_t sockctr = 0;
    int i;

    read_imc_counter_batch(act);
    read_imc_counter_batch(pre);
    read_imc_counter_batch(cas);
    switch (act)
    {
        case 0:
            actr = pcd->ctr0;
            break;
        case 1:
            actr = pcd->ctr1;
            break;
        case 2:
            actr = pcd->ctr2;
            break;
        case 3:
            actr = pcd->ctr3;
            break;
        case 4:
            actr = pcd->ctr4;
            break;
    }
    switch (pre)
    {
        case 0:
            pctr = pcd->ctr0;
            break;
        case 1:
            pctr = pcd->ctr1;
            break;
        case 2:
            pctr = pcd->ctr2;
            break;
        case 3:
            pctr = pcd->ctr3;
            break;
        case 4:
            pctr = pcd->ctr4;
            break;
    }
    switch (cas)
    {
        case 0:
            cctr = pcd->ctr0;
            break;
        case 1:
            cctr = pcd->ctr1;
            break;
        case 2:
            cctr = pcd->ctr2;
            break;
        case 3:
            cctr = pcd->ctr3;
            break;
        case 4:
            cctr = pcd->ctr4;
            break;
    }

    fprintf(writedest, "Percent Requests Caused Page Empty\n");
    for (i = 0; i < NUMCTRS * num_sockets(); i++)
    {
        if (i > 0 && i%4 == 0)
        {
            devctr++;
        }
        devctr %= 2;
        func %= 4;
        if (i > 0 && i%8 == 0)
        {
            sockctr++;
        }
        fprintf(writedest, "dev %d func %d sock %d: ", devnums[devctr], funcnums[func], sockctr);
        fprintf(writedest, "%lf\n", (double)(*actr[i] - *pctr[i]) / (*cctr[i] ? *cctr[i] : 1));
        func++;
    }
    return 0;
}

int print_mem_page_miss_from_ctr(const unsigned pre, const unsigned cas, FILE *writedest)
{
    struct pmonctrs_data *pcd = pmon_ctr_storage();
    uint64_t **pctr = NULL;
    uint64_t **cctr = NULL;
    const uint8_t funcnums[4] = {IMC_CH0_FUNC, IMC_CH1_FUNC, IMC_CH2_FUNC, IMC_CH3_FUNC};
    const uint8_t devnums[2] = {IMC0_DEV, IMC1_DEV};
    uint8_t devctr = 0;
    uint8_t func = 0;
    uint8_t sockctr = 0;
    int i;

    read_imc_counter_batch(pre);
    read_imc_counter_batch(cas);
    switch (pre)
    {
        case 0:
            pctr = pcd->ctr0;
            break;
        case 1:
            pctr = pcd->ctr1;
            break;
        case 2:
            pctr = pcd->ctr2;
            break;
        case 3:
            pctr = pcd->ctr3;
            break;
        case 4:
            pctr = pcd->ctr4;
            break;
    }
    switch (cas)
    {
        case 0:
            cctr = pcd->ctr0;
            break;
        case 1:
            cctr = pcd->ctr1;
            break;
        case 2:
            cctr = pcd->ctr2;
            break;
        case 3:
            cctr = pcd->ctr3;
            break;
        case 4:
            cctr = pcd->ctr4;
            break;
    }

    fprintf(writedest, "Percent Requests Caused Page Miss\n");
    for (i = 0; i < NUMCTRS * num_sockets(); i++)
    {
        if (i > 0 && i%4 == 0)
        {
            devctr++;
        }
        devctr %= 2;
        func %= 4;
        if (i > 0 && i%8 == 0)
        {
            sockctr++;
        }
        fprintf(writedest, "dev %d func %d sock %d: ", devnums[devctr], funcnums[func], sockctr);
        fprintf(writedest, "%lf\n", (double)*pctr[i] / (*cctr[i] ? *cctr[i] : 1));
        func++;
    }
    return 0;
}

int print_pmon_ctrs(void)
{
    struct pmonctrs_data *pcd = pmon_ctr_storage();
    const uint8_t funcnums[4] = {IMC_CH0_FUNC, IMC_CH1_FUNC, IMC_CH2_FUNC, IMC_CH3_FUNC};
    const uint8_t devnums[2] = {IMC0_DEV, IMC1_DEV};
    uint8_t devctr = 0;
    uint8_t func = 0;
    uint8_t sockctr = 0;
    int i;

    if (init_pmon_ctrs() > 0)
    {
        libmsr_error_handler("print_pmon_ctrs(): CSR iMC PMON counters have not been initialized", LIBMSR_ERROR_CSR_INIT, getenv("HOSTNAME"), __FILE__, __LINE__);
        return -1;
    }
    do_csr_batch_op(CSR_IMC_CTRS);

    for (i = 0; i < num_sockets() * NUMCTRS; i++)
    {
        if (i > 0 && i%4 == 0)
        {
            devctr++;
        }
        devctr %= 2;
        func %= 4;
        if (i > 0 && i%8 == 0)
        {
            sockctr++;
        }
        fprintf(stdout, "dev %d func %d sock %d\n", devnums[devctr], funcnums[func], sockctr);
        fprintf(stdout, "CTR0 %lx\n", *pcd->ctr0[i]);
        fprintf(stdout, "CTR1 %lx\n", *pcd->ctr1[i]);
        fprintf(stdout, "CTR2 %lx\n", *pcd->ctr2[i]);
        fprintf(stdout, "CTR3 %lx\n", *pcd->ctr3[i]);
        fprintf(stdout, "CTR4 %lx\n", *pcd->ctr4[i]);
        func++;
    }
    return 0;
}
