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
 * Science, under Award number XXXXX.
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

#define CSRDEBUG
#define FILENAME_SIZE 128
#define MODULE "/dev/cpu/csr_safe"
#define NUMCTRS 8

static int load_imc_batch_for_each(const size_t offt, uint64_t **loc, 
	const int isread, const size_t size, const unsigned batchno)
{
		int idx = 0;
		if (loc) {
			create_csr_batch_op(offt, 1, 16, 0, 0, isread, size, &loc[idx], batchno);
			create_csr_batch_op(offt, 1, 16, 1, 0, isread, size, &loc[++idx], batchno);
			create_csr_batch_op(offt, 1, 16, 4, 0, isread, size, &loc[++idx], batchno);
			create_csr_batch_op(offt, 1, 16, 5, 0, isread, size, &loc[++idx], batchno);

			create_csr_batch_op(offt, 1, 30, 0, 0, isread, size, &loc[++idx], batchno);
			create_csr_batch_op(offt, 1, 30, 1, 0, isread, size, &loc[++idx], batchno);
			create_csr_batch_op(offt, 1, 30, 4, 0, isread, size, &loc[++idx], batchno);
			create_csr_batch_op(offt, 1, 30, 5, 0, isread, size, &loc[++idx], batchno);

			if (num_sockets() > 1) {
				create_csr_batch_op(offt, 1, 16, 0, 1, isread, size, &loc[++idx], batchno);
				create_csr_batch_op(offt, 1, 16, 1, 1, isread, size, &loc[++idx], batchno);
				create_csr_batch_op(offt, 1, 16, 4, 1, isread, size, &loc[++idx], batchno);
				create_csr_batch_op(offt, 1, 16, 5, 1, isread, size, &loc[++idx], batchno);

				create_csr_batch_op(offt, 1, 30, 0, 1, isread, size, &loc[++idx], batchno);
				create_csr_batch_op(offt, 1, 30, 1, 1, isread, size, &loc[++idx], batchno);
				create_csr_batch_op(offt, 1, 30, 4, 1, isread, size, &loc[++idx], batchno);
				create_csr_batch_op(offt, 1, 30, 5, 1, isread, size, &loc[++idx], batchno);
			}
		} else {
			fprintf(stderr, "%s %s::%d ERROR: unable to create imc batch. loc was NULL\n",
					getenv("HOSTNAME"), __FILE__, __LINE__);
		}
		return idx;
}

struct pmonctrs_data *pmon_ctr_storage()
{
	static struct pmonctrs_data pcd;
	static int init = 1;
	if (init) {
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
		init = 0;
	}
	return &pcd;
}

struct pmonctr_global *pmonctr_global_storage()
{
	static struct pmonctr_global pgd;
	static int init = 1;
	if (init) {
		int allocated = NUMCTRS * num_sockets();
		pgd.unitctrl = (uint64_t **) libmsr_calloc(allocated, sizeof(uint64_t *));
		pgd.unitstatus = (uint64_t **) libmsr_calloc(allocated, sizeof(uint64_t *));
		init = 0;
	}
	return &pgd;
};

int init_pmon_ctrs()
{
	static int init = 1;
	static int allocated = 0;
	static struct pmonctrs_data *pmonctrs;
	if (init) {
		init = 0;
		allocated = NUMCTRS * num_sockets();
		pmonctrs = pmon_ctr_storage();
		allocate_csr_batch(CSR_IMC_PMON0, allocated);
		allocate_csr_batch(CSR_IMC_PMON1, allocated);
		allocate_csr_batch(CSR_IMC_PMON2, allocated);
		allocate_csr_batch(CSR_IMC_PMON3, allocated);
		allocate_csr_batch(CSR_IMC_PMON4, allocated);

		allocate_csr_batch(CSR_IMC_PMONCFG0, allocated);
		allocate_csr_batch(CSR_IMC_PMONCFG1, allocated);
		allocate_csr_batch(CSR_IMC_PMONCFG2, allocated);
		allocate_csr_batch(CSR_IMC_PMONCFG3, allocated);
		allocate_csr_batch(CSR_IMC_PMONCFG4, allocated);

		load_imc_batch_for_each(0xA0, pmonctrs->ctr0, 1, 6, CSR_IMC_PMON0);
		load_imc_batch_for_each(0xA8, pmonctrs->ctr1, 1, 6, CSR_IMC_PMON1);
		load_imc_batch_for_each(0xB0, pmonctrs->ctr2, 1, 6, CSR_IMC_PMON2);
		load_imc_batch_for_each(0xB8, pmonctrs->ctr3, 1, 6, CSR_IMC_PMON3);
		load_imc_batch_for_each(0xC0, pmonctrs->ctr4, 1, 6, CSR_IMC_PMON4);

		load_imc_batch_for_each(0xD8, pmonctrs->ctrcfg0, 0, 4, CSR_IMC_PMONCFG0);
		load_imc_batch_for_each(0xDC, pmonctrs->ctrcfg1, 0, 4, CSR_IMC_PMONCFG1);
		load_imc_batch_for_each(0xE0, pmonctrs->ctrcfg2, 0, 4, CSR_IMC_PMONCFG2);
		load_imc_batch_for_each(0xE4, pmonctrs->ctrcfg3, 0, 4, CSR_IMC_PMONCFG3);
		load_imc_batch_for_each(0xE8, pmonctrs->ctrcfg4, 0, 4, CSR_IMC_PMONCFG4);

		return 10 * allocated;
	}
	return -1;
}

int init_pmonctr_global()
{
	static int init = 1;
	static int allocated = 0;
	static struct pmonctr_global *pgd;
	if (init) {
		init = 0;
		allocated = NUMCTRS * num_sockets();
		pgd = pmonctr_global_storage();
		allocate_csr_batch(CSR_IMC_PMONUNITCTRL, allocated);
		allocate_csr_batch(CSR_IMC_PMONUNITSTAT, allocated);

		load_imc_batch_for_each(0xF4, pgd->unitctrl, 0, 3, CSR_IMC_PMONUNITCTRL);
		load_imc_batch_for_each(0xF8, pgd->unitstatus, 0, 1, CSR_IMC_PMONUNITSTAT);

		return 2 * allocated;
	}
	return -1;
}

static int set_pmon_config(const uint32_t setting, uint64_t **cfg)
{
	if (!cfg) {
		fprintf(stderr, "%s %s::%d ERROR: imc pmon cfg is not initialized\n",
				getenv("HOSTNAME"), __FILE__, __LINE__);
		return -1;
	}
	int i;
	for (i = 0; i < NUMCTRS * num_sockets(); i++) {
		*cfg[i] = setting;
	}
	return 0;
}

int pmon_config(uint32_t threshold, uint32_t invert, uint32_t ovf_en, uint32_t edge_det,
	uint32_t umask, uint8_t event, const unsigned counter)
{
	struct pmonctrs_data *pcd = pmon_ctr_storage();
	int ret0 = 0;
	int ret1 = 0;
	invert &= 0x1;
	ovf_en &= 0x1;
	edge_det &= 0x1;
	uint32_t setting = 0x0 | (threshold << 24) | (invert << 23) | (0x1 << 22) | 
			  (ovf_en << 20) | (edge_det << 18) | (0x1 << 17) | (0x1 << 16) |
			  (umask << 8) | event;
#ifdef CSRDEBUG
	fprintf(stderr, "CSRDEBUG: counter %u setting %x\n", counter, setting);
#endif
	switch (counter)
	{
		case 0:
			ret0 = set_pmon_config(setting, pcd->ctrcfg0);
			ret1 = do_csr_batch_op(CSR_IMC_PMONCFG0);
			break;
		case 1:
			ret0 = set_pmon_config(setting, pcd->ctrcfg1);
			ret1 = do_csr_batch_op(CSR_IMC_PMONCFG1);
			break;
		case 2:
			ret0 = set_pmon_config(setting, pcd->ctrcfg2);
			ret1 = do_csr_batch_op(CSR_IMC_PMONCFG2);
			break;
		case 3:
			ret0 = set_pmon_config(setting, pcd->ctrcfg3);
			ret1 = do_csr_batch_op(CSR_IMC_PMONCFG3);
			break;
		case 4:
			ret0 = set_pmon_config(setting, pcd->ctrcfg4);
			ret1 = do_csr_batch_op(CSR_IMC_PMONCFG4);
			break;
		default:
			fprintf(stderr, "%s %s::%d ERROR: imc perfmon counter %u does not exist\n",
					getenv("HOSTNAME"), __FILE__, __LINE__, counter);
			return -1;
	}
	return ret0 | ret1;
}

int set_pmon_unit_ctrl(uint32_t ovf_en, uint16_t freeze_en, uint16_t freeze, uint16_t reset,
	uint8_t reset_cfg)
{
	struct pmonctr_global *pgd = pmonctr_global_storage();
	uint32_t setting = 0x0 | (ovf_en << 17) | (freeze_en << 16) | (freeze << 8) |
					(reset << 1) | reset_cfg;
	int i;
	for (i = 0; i < NUMCTRS * num_sockets(); i++) {
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
			res = pmon_config(0x0, 0x0, 0x0, 0x0, UMASK_CAS_RD, EVT_CAS_COUNT, counter);
			break;
		case 1:
			res = pmon_config(0x0, 0x0, 0x0, 0x0, UMASK_CAS_WR, EVT_CAS_COUNT, counter);
			break;
		case 2:
			res = pmon_config(0x0, 0x0, 0x0, 0x0, UMASK_CAS_ALL, EVT_CAS_COUNT, counter);
			break;
		default:
			fprintf(stderr, "%s %s::%d ERROR: invalid bandwidth measurement specifier\n",
					getenv("HOSTNAME"), __FILE__, __LINE__);
			return -1;
	}
	return 0;
}

int get_mem_bw_from_ctr(const unsigned counter)
{
	int res = 0;
	switch (counter) {
		case 0:
			res = do_csr_batch_op(CSR_IMC_PMON0);
			break;
		case 1:
			res = do_csr_batch_op(CSR_IMC_PMON1);
			break;
		case 2:
			res = do_csr_batch_op(CSR_IMC_PMON2);
			break;
		case 3:
			res = do_csr_batch_op(CSR_IMC_PMON3);
			break;
		case 4:
			res = do_csr_batch_op(CSR_IMC_PMON4);
			break;
		default:
			fprintf(stderr, "%s %s::%d ERROR: imc pmon counter %d does not exist\n",
					getenv("HOSTNAME"), __FILE__, __LINE__, counter);
			return -1;
			break;
	}
	return res;
}

int print_mem_bw_from_ctr(const unsigned counter)
{
	struct pmonctrs_data *pcd = pmon_ctr_storage();
	uint64_t **ctr;
	get_mem_bw_from_ctr(counter);
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
	const uint8_t funcnums[4] = {0, 1, 4, 5};
	const uint8_t devnums[2] = {16, 30};
	uint8_t devctr = 0, func = 0, sockctr = 0;
	int i;
	for (i = 0; i < NUMCTRS * num_sockets(); i++) {
		if (i > 0 && i % 4 == 0) {
			devctr++;
		}
		devctr %= 2;
		func %= 4;
		if (i > 0 && i % 8 == 0) {
			sockctr++;
		}
		fprintf(stdout, "dev %d func %d sock %d\n", devnums[devctr], funcnums[func], sockctr);
		fprintf(stdout, "BW CTR%d %lld bytes\n", counter, *pcd->ctr0[i] * 64LL);
		func++;
	}
	return 0;
}

int print_pmon_ctrs()
{
	struct pmonctrs_data *pcd = pmon_ctr_storage();
	if (init_pmon_ctrs() > 0)
	{
        fprintf(stderr, "%s %s::%d ERROR: CSR imc pmon counters have not been initialized\n",
                getenv("HOSTNAME"), __FILE__, __LINE__);
	    return -1;
	}
	do_csr_batch_op(CSR_IMC_PMON0);
	do_csr_batch_op(CSR_IMC_PMON1);
	do_csr_batch_op(CSR_IMC_PMON2);
	do_csr_batch_op(CSR_IMC_PMON3);
	do_csr_batch_op(CSR_IMC_PMON4);
	const uint8_t funcnums[4] = {0, 1, 4, 5};
	const uint8_t devnums[2] = {16, 30};
	uint8_t devctr = 0, func = 0, sockctr = 0;
	int i;
	for (i = 0; i < num_sockets() * NUMCTRS; i++) {
		if (i > 0 && i % 4 == 0) {
			devctr++;
		}
		devctr %= 2;
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
		func++;
	}
	return 0;
}
