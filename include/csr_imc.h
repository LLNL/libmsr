/* csr_imc.h
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

#include <linux/types.h>
#include "master.h"

#ifdef __cplusplus
extern "C" {
#endif

enum{
	READ_BW,
	WRITE_BW,
	ALL_BW
};

enum{
	READ_PCT,
	WRITE_PCT
};

struct imc_ctrs_data
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

struct imc_fixed_data
{
	uint64_t **fctr;
	// config
	uint64_t **fctrcfg;
};

struct imc_global_data
{
	uint64_t **unitctrl;
	uint64_t **unitstatus;
};

 struct imc_ctrs_data *imc_ctr_storage();
 int init_imc_ctrs();
 struct imc_global_data *imc_global_storage();
 int init_imc_global();
 int set_imc_unit_ctrl(uint32_t ovf_en, uint16_t freeze_en, uint16_t freeze, uint16_t reset, uint8_t reset_cfg);
 int imc_ctr_config(uint32_t threshold, uint32_t ovf_en, uint32_t edge_det,
 	uint32_t umask, uint8_t event, const unsigned counter);
 int mem_bw_on_ctr(const unsigned counter, const int type);
 int get_mem_bw_from_ctr(const unsigned counter);
 int mem_pct_rw_on_ctr(const unsigned rcounter, const unsigned wcounter);
 int mem_page_empty_on_ctr(const unsigned act_count, const unsigned pre_count, const unsigned cas_count);
 int mem_page_miss_on_ctr(const unsigned pre_count, const unsigned cas_count);
 int read_imc_counter_batch(const unsigned counter);
 int dump_mem_bw_from_ctr(const unsigned counter, FILE * dest);
 int dump_mem_pct_rw_from_ctr(const unsigned rcounter, const unsigned wcounter, int type, FILE *dest);
 int dump_mem_bw_from_ctr_terse(const unsigned counter, FILE * dest);
 int dump_mem_pct_rw_from_ctr_terse(const unsigned rcounter, const unsigned wcounter, int type, FILE *dest);
 int dump_mem_page_empty_from_ctr(const unsigned act, const unsigned pre, const unsigned cas, FILE * dest);
 int dump_mem_page_miss_from_ctr(const unsigned pre, const unsigned cas, FILE * dest);
 int dump_pmon_ctrs();

 #ifdef __cplusplus
 }
 #endif
