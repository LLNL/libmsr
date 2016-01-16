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
 * Science, under Award number xxxx.
 *
 */

#include <linux/types.h>

// These EVENTS are known to work with Ivy Bridge only
#define EVT_DCLOCKTICKS 0x00
#define EVT_ACT_COUNT 0x01
#define EVT_PRE_COUNT 0x02
#define EVT_CAS_COUNT 0x04
#define EVT_DRAM_REFRESH 0x05
#define EVT_DRAM_PRE_ALL 0x06
#define EVT_MAJOR_MODES 0x07
#define EVT_PREEMPTION 0x08
#define EVT_ECC_CORRECTABLE_ERRORS 0x09
#define EVT_RPQ_INSERTS 0x10
#define EVT_RPQ_CYCLES_NE 0x11
#define EVT_WPQ_INSERTS 0x20
#define EVT_WPQ_CYCLES_NE 0x21
#define EVT_WPQ_CYCLES_FULL 0x22
#define EVT_WPQ_READ_HIT 0x23
#define EVT_WPQ_WRITE_HIT 0x24
#define EVT_POWER_THROTTLE_CYCLES 0x41
#define EVT_POWER_PCU_THROTTLING 0x42
#define EVT_POWER_SELF_REFRESH 0x43
#define EVT_POWER_CKE_CYCLES 0x83
#define EVT_POWER_CHANNEL_DLLOFF 0x84
#define EVT_POWER_CHANNEL_PD 0x85
#define EVT_POWER_CRITICAL_THROTTLE_CYCLES 0x86
#define EVT_VMSE_WR_PUSH 0x90
#define EVT_VMSE_MXB_WR_OCCUPANCY 0x91
#define EVT_RD_CAS_PRIO	0xA0
#define EVT_BYP_CMDS 0xA1
#define EVT_RD_CAS_RANK0 0xB0 
#define EVT_RD_CAS_RANK1 0xB1 
#define EVT_RD_CAS_RANK2 0xB2 
#define EVT_RD_CAS_RANK3 0xB3 
#define EVT_RD_CAS_RANK4 0xB4 
#define EVT_RD_CAS_RANK5 0xB5 
#define EVT_RD_CAS_RANK6 0xB6 
#define EVT_RD_CAS_RANK7 0xB7 
#define EVT_WR_CAS_RANK0 0xB8 
#define EVT_WR_CAS_RANK1 0xB9 
#define EVT_WR_CAS_RANK2 0xBA 
#define EVT_WR_CAS_RANK3 0xBB 
#define EVT_WR_CAS_RANK4 0xBC 
#define EVT_WR_CAS_RANK5 0xBD 
#define EVT_WR_CAS_RANK6 0xBE 
#define EVT_WR_CAS_RANK7 0xBF 
#define EVT_WMM_TO_RMM 0xC0
#define EVT_WRONG_MM 0xC1

// These UMASKS are known to work with Ivy Bridge only
#define UMASK_CAS_RD_REG 0x1
#define UMASK_CAS_RD_UNDERFILL 0x2
#define UMASK_CAS_RD 0x3
#define UMASK_CAS_WR_WMM 0x4
#define UMASK_CAS_WR_RMM 0x8
#define UMASK_CAS_WR 0xC
#define UMASK_CAS_ALL 0xF
#define UMASK_CAS_RD_WMM 0x16
#define UMASK_CAS_RD_RMM 0x32

#define UMASK_ACT_COUNT_RD 0x1
#define UMASK_ACT_COUNT_WR 0x2
#define UMASK_ACT_COUNT_BYP 0x4

#define UMASK_BYP_CMDS_ACT 0x1
#define UMASK_BYP_CMDS_CAS 0x2
#define UMASK_BYP_CMDS_PRE 0x4
// ... there are lots more of these...

enum{
	READ_BW,
	WRITE_BW,
	ALL_BW
};

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

struct fixed_perfmon_data
{
	uint64_t **fctr;
	// config
	uint64_t **fctrcfg;
};

struct pmonctr_global
{
	uint64_t **unitctrl;
	uint64_t **unitstatus;
};

 struct pmonctrs_data *pmon_ctr_storage();
 int init_pmon_ctrs();
 int pmon_config(uint32_t threshold, uint32_t invert, uint32_t ovf_en, uint32_t edge_det,
 	uint32_t umask, uint8_t event, const unsigned counter);
 int mem_bw_on_ctr(const unsigned counter, const int type);
 int get_mem_bw_from_ctr(const unsigned counter);
 int print_mem_bw_from_ctr(const unsigned counter);
 int print_pmon_ctrs();
