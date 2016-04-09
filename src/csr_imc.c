/* csr_imc.c
 *
 * Copyright (c) 2011-2016, Lawrence Livermore National Security, LLC. 
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

 /* This file contains iMC related CSRs */

#include <sys/stat.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>

#include "csr_core.h"
#include "memhdlr.h"
#include "msr_core.h"
#include "csr_imc.h"

//#define CSRDEBUG
#define FILENAME_SIZE 128
#define MODULE "/dev/cpu/csr_safe"

static int load_imc_batch_for_each(const size_t offt, uint64_t **loc, 
	const int isread, const size_t size, const unsigned batchno)
{
		int idx = 0;
		if (loc) {
			// this macro changes depending on architecture
			LOAD_IMC_BATCH(offt, loc, isread, size, batchno);
		} else {
			fprintf(stderr, "%s %s::%d ERROR: unable to create imc batch. loc was NULL\n",
					getenv("HOSTNAME"), __FILE__, __LINE__);
		}
		return idx;
}

struct imc_ctrs_data *imc_ctr_storage()
{
	static struct imc_ctrs_data pcd;
	static int init = 1;
	if (init) {
		int allocated = IMC_NUMCTRS * num_sockets();
		pcd.ctr0 = (uint64_t **) libmsr_calloc(allocated, sizeof(uint64_t *));
		pcd.ctr1 = (uint64_t **) libmsr_calloc(allocated, sizeof(uint64_t *));
		pcd.ctr2 = (uint64_t **) libmsr_calloc(allocated, sizeof(uint64_t *));
		pcd.ctr3 = (uint64_t **) libmsr_calloc(allocated, sizeof(uint64_t *));
		// config
		pcd.ctrcfg0 = (uint64_t **) libmsr_calloc(allocated, sizeof(uint64_t *));
		pcd.ctrcfg1 = (uint64_t **) libmsr_calloc(allocated, sizeof(uint64_t *));
		pcd.ctrcfg2 = (uint64_t **) libmsr_calloc(allocated, sizeof(uint64_t *));
		pcd.ctrcfg3 = (uint64_t **) libmsr_calloc(allocated, sizeof(uint64_t *));
		init = 0;
	}
	return &pcd;
}

struct imc_global_data *imc_global_storage()
{
	static struct imc_global_data pgd;
	static int init = 1;
	if (init) {
		int allocated = IMC_NUMCTRS * num_sockets();
		pgd.unitctrl = (uint64_t **) libmsr_calloc(allocated, sizeof(uint64_t *));
		pgd.unitstatus = (uint64_t **) libmsr_calloc(allocated, sizeof(uint64_t *));
		init = 0;
	}
	return &pgd;
};

int init_imc_ctrs()
{
	static int init = 1;
	static int allocated = 0;
	static struct imc_ctrs_data *pmonctrs;
	if (init) {
		init = 0;
		allocated = IMC_NUMCTRS * num_sockets();
		pmonctrs = imc_ctr_storage();

		// allocated * 4 because putting all 4 counters in the same batch
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

		return 8 * allocated;
	}
	return -1;
}

int init_imc_global()
{
	static int init = 1;
	static int allocated = 0;
	static struct imc_global_data *pgd;
	if (init) {
		init = 0;
		allocated = IMC_NUMCTRS * num_sockets();
		pgd = imc_global_storage();
		allocate_csr_batch(CSR_IMC_PMONUNITCTRL, allocated);
		allocate_csr_batch(CSR_IMC_PMONUNITSTAT, allocated);

		load_imc_batch_for_each(CSR_PMONUNITCTRL, pgd->unitctrl, 0, 3, CSR_IMC_PMONUNITCTRL);
		load_imc_batch_for_each(CSR_PMONUNITSTAT, pgd->unitstatus, 0, 1, CSR_IMC_PMONUNITSTAT);

		return 2 * allocated;
	}
	return -1;
}

static int set_pmon_config(const uint32_t setting, uint64_t **cfg)
{
	if (!*cfg) {
		fprintf(stderr, "%s %s::%d ERROR: imc pmon cfg is not initialized\n",
				getenv("HOSTNAME"), __FILE__, __LINE__);
		return -1;
	}
	int i;
	for (i = 0; i < IMC_NUMCTRS * num_sockets(); i++) {
		*cfg[i] = setting;
	}
	return 0;
}

int imc_ctr_config(uint32_t threshold, uint32_t ovf_en, uint32_t edge_det,
	uint32_t umask, uint8_t event, const unsigned counter)
{
	struct imc_ctrs_data *pcd = imc_ctr_storage();
	int ret0 = 0;
	int ret1 = 0;
	ovf_en &= 0x1;
	edge_det &= 0x1;
	uint32_t setting = 0x0 | (threshold << 24) | (0x1 << 22) | 
			  (ovf_en << 20) | (edge_det << 18) | (0x1 << 17) | (0x1 << 16) |
			  (umask << 8) | event;
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
			fprintf(stderr, "%s %s::%d ERROR: imc perfmon counter %u does not exist\n",
					getenv("HOSTNAME"), __FILE__, __LINE__, counter);
			return -1;
	}
	return ret0 | ret1;
}

int set_imc_unit_ctrl(uint32_t ovf_en, uint16_t freeze_en, uint16_t freeze, uint16_t reset,
	uint8_t reset_cfg)
{
	struct imc_global_data *pgd = imc_global_storage();
	uint32_t setting = 0x0 | (ovf_en << 17) | (freeze_en << 16) | (freeze << 8) |
					(reset << 1) | reset_cfg;
	int i;
	for (i = 0; i < IMC_NUMCTRS * num_sockets(); i++) {
		*pgd->unitctrl[i] = setting;
	}
	return do_csr_batch_op(CSR_IMC_PMONUNITCTRL);
}

int mem_bw_on_ctr(const unsigned counter, const int type)
{
	if (counter > 4) {
		fprintf(stderr, "%s %s::%d ERROR: imc pmon counter %d does not exist\n",
				getenv("HOSTNAME"), __FILE__, __LINE__, counter);
		return -1;
	}
	int res = 0; 
	switch (type) {
		case 0:
			res = imc_ctr_config(0x0, 0x0, 0x0, UMASK_CAS_RD, EVT_CAS_COUNT, counter);
			break;
		case 1:
			res = imc_ctr_config(0x0, 0x0, 0x0, UMASK_CAS_WR, EVT_CAS_COUNT, counter);
			break;
		case 2:
			res = imc_ctr_config(0x0, 0x0, 0x0, UMASK_CAS_ALL, EVT_CAS_COUNT, counter);
			break;
		default:
			fprintf(stderr, "%s %s::%d ERROR: invalid bandwidth measurement specifier\n",
					getenv("HOSTNAME"), __FILE__, __LINE__);
			return -1;
	}
	return res;
}

int mem_pct_rw_on_ctr(const unsigned rcounter, const unsigned wcounter)
{
	if (rcounter > 4 || wcounter > 4) {
		fprintf(stderr, "%s %s::%d ERROR: imc pmon counter %d does not exist\n",
				getenv("HOSTNAME"), __FILE__, __LINE__, (wcounter > rcounter ? wcounter : rcounter));
		return -1;
	}
	if (rcounter == wcounter) {
		fprintf(stderr, "%s %s::%d ERROR: you can't count different events on the same register (%d)\n",
				getenv("HOSTNAME"), __FILE__, __LINE__, wcounter);
	}
	int res = 0; 
	res |= imc_ctr_config(0x0, 0x0, 0x0, 0x0, EVT_RPQ_INSERTS, rcounter);
	res |= imc_ctr_config(0x0, 0x0, 0x0, 0x0, EVT_WPQ_INSERTS, wcounter);
	return res;
}

int mem_page_empty_on_ctr(const unsigned act_count, const unsigned pre_count, const unsigned cas_count)
{
	if (act_count > 4 || pre_count > 4 || cas_count > 4) {
		fprintf(stderr, "%s %s::%d ERROR: there are only 4 IMC performance counters. Had (%d, %d, %d)\n",
				getenv("HOSTNAME"), __FILE__, __LINE__, act_count, pre_count, cas_count);
		return -1;
	}
	if (act_count == pre_count || cas_count == pre_count || cas_count == act_count) {
		fprintf(stderr, "%s %s::%d ERROR: you can't count different events on the same register. Had (%d, %d, %d)\n",
				getenv("HOSTNAME"), __FILE__, __LINE__, act_count, pre_count, cas_count);
	}
	int res = 0;
	res |= imc_ctr_config(0x0, 0x0, 0x0, UMASK_CAS_ALL, EVT_CAS_COUNT, cas_count);
	res |= imc_ctr_config(0x0, 0x0, 0x0, 0xFF, EVT_ACT_COUNT, act_count);
	res |= imc_ctr_config(0x0, 0x0, 0x0, UMASK_PRE_PAGE_MISS, EVT_PRE_COUNT, pre_count);
	return res;
}

int mem_page_miss_on_ctr(const unsigned pre_count, const unsigned cas_count)
{
	if (pre_count > 4 || pre_count > 4) {
		fprintf(stderr, "%s %s::%d ERROR: imc pmon counter %d does not exist\n",
				getenv("HOSTNAME"), __FILE__, __LINE__, (pre_count > cas_count ? pre_count : cas_count));
		return -1;
	}
	if (pre_count == cas_count) {
		fprintf(stderr, "%s %s::%d ERROR: you can't count different events on the same register (%d).\n",
				getenv("HOSTNAME"), __FILE__, __LINE__, pre_count);
	}
	int res = 0;
	res |= imc_ctr_config(0x0, 0x0, 0x0, UMASK_PRE_PAGE_MISS, EVT_PRE_COUNT, pre_count);
	res |= imc_ctr_config(0x0, 0x0, 0x0, UMASK_CAS_ALL, EVT_CAS_COUNT, cas_count);
	return res;
}

int read_imc_counter_batch(const unsigned counter)
{
	int res = 0;
	// TODO: counters have been consolidated, collapse this
	res = do_csr_batch_op(CSR_IMC_CTRS);
	return res;
}

int dump_mem_bw_from_ctr(const unsigned counter, FILE * dest)
{
	struct imc_ctrs_data *pcd = imc_ctr_storage();
	uint64_t **ctr = NULL;
	read_imc_counter_batch(counter);
	switch (counter) {
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
	const uint8_t funcnums[4] = {IMC_CH0_FUNC, IMC_CH1_FUNC, IMC_CH2_FUNC, IMC_CH3_FUNC};
	const uint8_t devnums[8] = {IMC0_DEV_1, IMC0_DEV_2, IMC0_DEV_3, IMC0_DEV_4, 
								IMC1_DEV_1, IMC1_DEV_2, IMC1_DEV_3, IMC1_DEV_4};
	uint8_t devctr = 0, func = 0, sockctr = 0;
	int i;
	fprintf(dest, "Memory Bandwidth\n");
	for (i = 0; i < IMC_NUMCTRS * num_sockets(); i++) {
		devctr %= 8;
		func %= 4;
		if (i > 0 && i % 8 == 0) {
			sockctr++;
		}
		fprintf(dest, "dev %d func %d sock %d: ", devnums[devctr], funcnums[func], sockctr);
		fprintf(dest, "%lu bytes\n", *ctr[i] * 64LU);
		func++;
		devctr++;
	}
	return 0;
}

int dump_mem_bw_from_ctr_terse(const unsigned counter, FILE * dest)
{
	struct imc_ctrs_data *pcd = imc_ctr_storage();
	uint64_t **ctr = NULL;
	read_imc_counter_batch(counter);
	switch (counter) {
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
	int i;
	for (i = 0; i < IMC_NUMCTRS * num_sockets(); i++) {
		fprintf(dest, "%lu\t", *ctr[i] * 64LU);
	}
	return 0;
}

int dump_mem_pct_rw_from_ctr(const unsigned rcounter, const unsigned wcounter, int type, FILE * dest)
{
	struct imc_ctrs_data *pcd = imc_ctr_storage();
	uint64_t **rctr = NULL, **wctr = NULL;
	read_imc_counter_batch(rcounter);
	read_imc_counter_batch(wcounter);
	switch (rcounter) {
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
	switch (wcounter) {
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
	const uint8_t funcnums[4] = {IMC_CH0_FUNC, IMC_CH1_FUNC, IMC_CH2_FUNC, IMC_CH3_FUNC};
	const uint8_t devnums[8] = {IMC0_DEV_1, IMC0_DEV_2, IMC0_DEV_3, IMC0_DEV_4, 
								IMC1_DEV_1, IMC1_DEV_2, IMC1_DEV_3, IMC1_DEV_4};
	uint8_t devctr = 0, func = 0, sockctr = 0;
	int i;
	fprintf(dest, "Percent %s Requests\n", (type ? "read\0" : "write\0"));
	for (i = 0; i < IMC_NUMCTRS * num_sockets(); i++) {
		devctr %= 8;
		func %= 4;
		if (i > 0 && i % 8 == 0) {
			sockctr++;
		}
		fprintf(dest, "dev %d func %d sock %d: ", devnums[devctr], funcnums[func], sockctr);
		if (type) {
			fprintf(dest, "%lf\n", (double) *rctr[i] / ((*rctr[i] + *wctr[i]) ? (*rctr[i] + *wctr[i]) : 1));
		} else {
			fprintf(dest, "%lf\n", (double) *wctr[i] / ((*rctr[i] + *wctr[i]) ? (*rctr[i] + *wctr[i]) : 1));
		}
		func++;
		devctr++;
	}
	return 0;
}

int dump_mem_pct_rw_from_ctr_terse(const unsigned rcounter, const unsigned wcounter, int type, FILE * dest)
{
	struct imc_ctrs_data *pcd = imc_ctr_storage();
	uint64_t **rctr = NULL, **wctr = NULL;
	read_imc_counter_batch(rcounter);
	read_imc_counter_batch(wcounter);
	switch (rcounter) {
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
	switch (wcounter) {
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
	int i;
	for (i = 0; i < IMC_NUMCTRS * num_sockets(); i++) {
		if (type) {
			fprintf(dest, "%lf\t", (double) *rctr[i] / ((*rctr[i] + *wctr[i]) ? (*rctr[i] + *wctr[i]) : 1));
		} else {
			fprintf(dest, "%lf\t", (double) *wctr[i] / ((*rctr[i] + *wctr[i]) ? (*rctr[i] + *wctr[i]) : 1));
		}
	}
	return 0;
}

int dump_mem_page_empty_from_ctr(const unsigned act, const unsigned pre, const unsigned cas, FILE * dest)
{
	struct imc_ctrs_data *pcd = imc_ctr_storage();
	uint64_t **actr = NULL, **pctr = NULL, **cctr = NULL;
	read_imc_counter_batch(act);
	read_imc_counter_batch(pre);
	read_imc_counter_batch(cas);
	switch (act) {
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
	switch (pre) {
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
	switch (cas) {
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
	const uint8_t funcnums[4] = {IMC_CH0_FUNC, IMC_CH1_FUNC, IMC_CH2_FUNC, IMC_CH3_FUNC};
	const uint8_t devnums[8] = {IMC0_DEV_1, IMC0_DEV_2, IMC0_DEV_3, IMC0_DEV_4, 
								IMC1_DEV_1, IMC1_DEV_2, IMC1_DEV_3, IMC1_DEV_4};
	uint8_t devctr = 0, func = 0, sockctr = 0;
	int i;
	fprintf(dest, "Percent Requests Caused Page Empty\n");
	for (i = 0; i < IMC_NUMCTRS * num_sockets(); i++) {
		devctr %= 8;
		func %= 4;
		if (i > 0 && i % 8 == 0) {
			sockctr++;
		}
		fprintf(dest, "dev %d func %d sock %d: ", devnums[devctr], funcnums[func], sockctr);
		fprintf(dest, "%lf\n", (double) (*actr[i] - *pctr[i]) / (*cctr[i] ? *cctr[i] : 1));
		func++;
		devctr++;
	}
	return 0;
}

int dump_mem_page_miss_from_ctr(const unsigned pre, const unsigned cas, FILE * dest)
{
	struct imc_ctrs_data *pcd = imc_ctr_storage();
	uint64_t **pctr = NULL, **cctr = NULL;
	read_imc_counter_batch(pre);
	read_imc_counter_batch(cas);
	switch (pre) {
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
	switch (cas) {
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
	const uint8_t funcnums[4] = {IMC_CH0_FUNC, IMC_CH1_FUNC, IMC_CH2_FUNC, IMC_CH3_FUNC};
	const uint8_t devnums[8] = {IMC0_DEV_1, IMC0_DEV_2, IMC0_DEV_3, IMC0_DEV_4, 
								IMC1_DEV_1, IMC1_DEV_2, IMC1_DEV_3, IMC1_DEV_4};
	uint8_t devctr = 0, func = 0, sockctr = 0;
	int i;
	fprintf(dest, "Percent Requests Caused Page Miss\n");
	for (i = 0; i < IMC_NUMCTRS * num_sockets(); i++) {
		devctr %= 8;
		func %= 4;
		if (i > 0 && i % 8 == 0) {
			sockctr++;
		}
		fprintf(dest, "dev %d func %d sock %d: ", devnums[devctr], funcnums[func], sockctr);
		fprintf(dest, "%lf\n", (double) *pctr[i] / (*cctr[i] ? *cctr[i] : 1));
		devctr++;
		func++;
	}
	return 0;
}

int dump_imc_ctrs()
{
	struct imc_ctrs_data *pcd = imc_ctr_storage();
	if (init_imc_ctrs() > 0)
	{
        fprintf(stderr, "%s %s::%d ERROR: CSR imc pmon counters have not been initialized\n",
                getenv("HOSTNAME"), __FILE__, __LINE__);
	    return -1;
	}
	do_csr_batch_op(CSR_IMC_CTRS);
	const uint8_t funcnums[4] = {IMC_CH0_FUNC, IMC_CH1_FUNC, IMC_CH2_FUNC, IMC_CH3_FUNC};
	const uint8_t devnums[8] = {IMC0_DEV_1, IMC0_DEV_2, IMC0_DEV_3, IMC0_DEV_4, 
								IMC1_DEV_1, IMC1_DEV_2, IMC1_DEV_3, IMC1_DEV_4};
	uint8_t devctr = 0, func = 0, sockctr = 0;
	int i;
	for (i = 0; i < num_sockets() * IMC_NUMCTRS; i++) {
		devctr %= 8;
		func %= 4;
		if (i > 0 && i % 8 == 0) {
			sockctr++;
		}
		fprintf(stdout, "dev %d func %d sock %d\n", devnums[devctr], funcnums[func], sockctr);
		fprintf(stdout, "CTR0 %lx\n", *pcd->ctr0[i]);
		fprintf(stdout, "CTR1 %lx\n", *pcd->ctr1[i]);
		fprintf(stdout, "CTR2 %lx\n", *pcd->ctr2[i]);
		fprintf(stdout, "CTR3 %lx\n", *pcd->ctr3[i]);
		fprintf(stdout, "CTR4 %lx\n", *pcd->ctr4[i]);
		devctr++;
		func++;
	}
	return 0;
}
