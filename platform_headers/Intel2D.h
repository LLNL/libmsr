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

#define COMPILED_VEND 0x8086 // Intel
#define COMPILED_ARCH 0x2D   // Sandy Bridge
#define IS_ARCH_2D 1

/**********/
/* CLOCKS */
/**********/
#define IA32_MPERF              0xE7
#define IA32_APERF              0xE8
#define IA32_TIME_STAMP_COUNTER 0x10
#define IA32_CLOCK_MODULATION   0x19A
#define IA32_PERF_STATUS        0x198
#define IA32_PERF_CTL           0x199

/************/
/* COUNTERS */
/************/
#define IA32_FIXED_CTR_CTRL       0x38D // Controls for fixed ctr0, 1, and 2
#define IA32_PERF_GLOBAL_CTRL     0x38F // Enables for fixed ctr0,1,and2 here
#define IA32_PERF_GLOBAL_STATUS   0x38E // Overflow condition can be found
                                        // here
#define IA32_PERF_GLOBAL_OVF_CTRL 0x390 // Can clear the overflow here
#define IA32_FIXED_CTR0           0x309 // (R/W) Counts Instr_Retired.Any
#define IA32_FIXED_CTR1           0x30A // (R/W) Counts CPU_CLK_Unhalted.Core
#define IA32_FIXED_CTR2           0x30B // (R/W) Counts CPU_CLK_Unhalted.Ref

#define IA32_PMC0 0xC1
#define IA32_PMC1 0xC2
#define IA32_PMC2 0xC3
#define IA32_PMC3 0xC4
#define IA32_PMC4 0xC5
#define IA32_PMC5 0xC6
#define IA32_PMC6 0xC7
#define IA32_PMC7 0xC8

#define IA32_PERFEVTSEL0 0x186
#define IA32_PERFEVTSEL1 0x187
#define IA32_PERFEVTSEL2 0x188
#define IA32_PERFEVTSEL3 0x189
#define IA32_PERFEVTSEL4 0x18A
#define IA32_PERFEVTSEL5 0x18B
#define IA32_PERFEVTSEL6 0x18C
#define IA32_PERFEVTSEL7 0x18D

#define MSR_PCU_PMON_EVNTSEL0 0xC30
#define MSR_PCU_PMON_EVNTSEL1 0xC31
#define MSR_PCU_PMON_EVNTSEL2 0xC32
#define MSR_PCU_PMON_EVNTSEL3 0xC33
#define MSR_PCU_PMON_CTR0     0xC36
#define MSR_PCU_PMON_CTR1     0xC37
#define MSR_PCU_PMON_CTR2     0xC38
#define MSR_PCU_PMON_CTR3     0xC39

#define MSR_C0_PMON_BOX_CTL  0xD04
#define MSR_C0_PMON_EVNTSEL0 0xD10
#define MSR_C0_PMON_EVNTSEL1 0xD11
#define MSR_C0_PMON_EVNTSEL2 0xD12
#define MSR_C0_PMON_EVNTSEL3 0xD13
#define MSR_C0_BOX_FILTER    0xD14
#define MSR_C0_PMON_CTR0     0xD16
#define MSR_C0_PMON_CTR1     0xD17
#define MSR_C0_PMON_CTR2     0xD18
#define MSR_C0_PMON_CTR3     0xD19

#define MSR_C1_PMON_BOX_CTL  0xD24
#define MSR_C1_PMON_EVNTSEL0 0xD30
#define MSR_C1_PMON_EVNTSEL1 0xD31
#define MSR_C1_PMON_EVNTSEL2 0xD32
#define MSR_C1_PMON_EVNTSEL3 0xD33
#define MSR_C1_BOX_FILTER    0xD34
#define MSR_C1_PMON_CTR0     0xD36
#define MSR_C1_PMON_CTR1     0xD37
#define MSR_C1_PMON_CTR2     0xD38
#define MSR_C1_PMON_CTR3     0xD39

#define MSR_C2_PMON_BOX_CTL  0xD44
#define MSR_C2_PMON_EVNTSEL0 0xD50
#define MSR_C2_PMON_EVNTSEL1 0xD51
#define MSR_C2_PMON_EVNTSEL2 0xD52
#define MSR_C2_PMON_EVNTSEL3 0xD53
#define MSR_C2_BOX_FILTER    0xD54
#define MSR_C2_PMON_CTR0     0xD56
#define MSR_C2_PMON_CTR1     0xD57
#define MSR_C2_PMON_CTR2     0xD58
#define MSR_C2_PMON_CTR3     0xD59

#define MSR_C3_PMON_BOX_CTL  0xD64
#define MSR_C3_PMON_EVNTSEL0 0xD70
#define MSR_C3_PMON_EVNTSEL1 0xD71
#define MSR_C3_PMON_EVNTSEL2 0xD72
#define MSR_C3_PMON_EVNTSEL3 0xD73
#define MSR_C3_BOX_FILTER    0xD74
#define MSR_C3_PMON_CTR0     0xD76
#define MSR_C3_PMON_CTR1     0xD77
#define MSR_C3_PMON_CTR2     0xD78
#define MSR_C3_PMON_CTR3     0xD79

#define MSR_C4_PMON_BOX_CTL  0xD84
#define MSR_C4_PMON_EVNTSEL0 0xD90
#define MSR_C4_PMON_EVNTSEL1 0xD91
#define MSR_C4_PMON_EVNTSEL2 0xD92
#define MSR_C4_PMON_EVNTSEL3 0xD93
#define MSR_C4_BOX_FILTER    0xD94
#define MSR_C4_PMON_CTR0     0xD96
#define MSR_C4_PMON_CTR1     0xD97
#define MSR_C4_PMON_CTR2     0xD98
#define MSR_C4_PMON_CTR3     0xD99

#define MSR_C5_PMON_BOX_CTL  0xDA4
#define MSR_C5_PMON_EVNTSEL0 0xDB0
#define MSR_C5_PMON_EVNTSEL1 0xDB1
#define MSR_C5_PMON_EVNTSEL2 0xDB2
#define MSR_C5_PMON_EVNTSEL3 0xDB3
#define MSR_C5_BOX_FILTER    0xDB4
#define MSR_C5_PMON_CTR0     0xDB6
#define MSR_C5_PMON_CTR1     0xDB7
#define MSR_C5_PMON_CTR2     0xDB8
#define MSR_C5_PMON_CTR3     0xDB9

#define MSR_C6_PMON_BOX_CTL  0xDC4
#define MSR_C6_PMON_EVNTSEL0 0xDD0
#define MSR_C6_PMON_EVNTSEL1 0xDD1
#define MSR_C6_PMON_EVNTSEL2 0xDD2
#define MSR_C6_PMON_EVNTSEL3 0xDD3
#define MSR_C6_BOX_FILTER    0xDD4
#define MSR_C6_PMON_CTR0     0xDD6
#define MSR_C6_PMON_CTR1     0xDD7
#define MSR_C6_PMON_CTR2     0xDD8
#define MSR_C6_PMON_CTR3     0xDD9

#define MSR_C7_PMON_BOX_CTL  0xDE4
#define MSR_C7_PMON_EVNTSEL0 0xDF0
#define MSR_C7_PMON_EVNTSEL1 0xDF1
#define MSR_C7_PMON_EVNTSEL2 0xDF2
#define MSR_C7_PMON_EVNTSEL3 0xDF3
#define MSR_C7_BOX_FILTER    0xDF4
#define MSR_C7_PMON_CTR0     0xDF6
#define MSR_C7_PMON_CTR1     0xDF7
#define MSR_C7_PMON_CTR2     0xDF8
#define MSR_C7_PMON_CTR3     0xDF9

/********/
/* MISC */
/********/
#define IA32_MISC_ENABLE      0x1A0
#define MSR_PKG_C2_RESIDENCY  0x60D
#define MSR_PKG_C3_RESIDENCY  0x3F8
#define MSR_PKG_C6_RESIDENCY  0x3F9
#define MSR_PKG_C7_RESIDENCY  0x3FA
#define MSR_CORE_C3_RESIDENCY 0x3FC
#define MSR_CORE_C6_RESIDENCY 0x3FD
#define MSR_CORE_C7_RESIDENCY 0x3FE

/********/
/* RAPL */
/********/
#define MSR_RAPL_POWER_UNIT    0x606 // ro
#define MSR_PKG_POWER_LIMIT    0x610 // rw
#define MSR_PKG_ENERGY_STATUS  0x611 // ro sic
#define MSR_PKG_POWER_INFO     0x614 // rw text states ro

#define MSR_PKG_PERF_STATUS    0x613 // ro
#define MSR_DRAM_POWER_LIMIT   0x618 // rw
#define MSR_DRAM_ENERGY_STATUS 0x619 // ro. sic
#define MSR_DRAM_PERF_STATUS   0x61B // ro
#define MSR_DRAM_POWER_INFO    0x61C // rw text states ro

/***********/
/* THERMAL */
/***********/
#define IA32_THERM_STATUS            0x19C
#define MSR_THERM2_CTL               0x19D
#define IA32_THERM_INTERRUPT         0x19B
#define IA32_PACKAGE_THERM_STATUS    0x1B1
#define IA32_PACKAGE_THERM_INTERRUPT 0x1B2
#define MSR_TEMPERATURE_TARGET       0x1A2

/*********/
/* TURBO */
/*********/
#define IA32_MISC_ENABLE 0x1A0
#define IA32_PERF_CTL    0x199

/***********/
/* CSR iMC */
/***********/
#define IMC0_DEV         16
#define IMC1_DEV         0
#define IMC_CH0_FUNC     0
#define IMC_CH1_FUNC     1
#define IMC_CH2_FUNC     4
#define IMC_CH3_FUNC     5

#define CSR_PMONCTRCFG0  0xD8
#define CSR_PMONCTRCFG1  0xDC
#define CSR_PMONCTRCFG2  0xE0
#define CSR_PMONCTRCFG3  0xE4

#define CSR_PMONCTR0     0xA0
#define CSR_PMONCTR1     0xA8
#define CSR_PMONCTR2     0xB0
#define CSR_PMONCTR3     0xB8

#define CSR_PMONUNITCTRL 0xF4
#define CSR_PMONUNITSTAT 0xF8

/******************/
/* CSR iMC EVENTS */
/******************/
#define EVT_DCLOCKTICKS                    0x00
#define EVT_ACT_COUNT                      0x01
#define EVT_PRE_COUNT                      0x02
#define EVT_CAS_COUNT                      0x04
#define EVT_DRAM_REFRESH                   0x05
#define EVT_DRAM_PRE_ALL                   0x06
#define EVT_MAJOR_MODES                    0x07
#define EVT_PREEMPTION                     0x08
#define EVT_ECC_CORRECTABLE_ERRORS         0x09
#define EVT_RPQ_INSERTS                    0x10
#define EVT_RPQ_CYCLES_NE                  0x11
#define EVT_WPQ_INSERTS                    0x20
#define EVT_WPQ_CYCLES_NE                  0x21
#define EVT_WPQ_CYCLES_FULL                0x22
#define EVT_WPQ_READ_HIT                   0x23
#define EVT_WPQ_WRITE_HIT                  0x24
#define EVT_POWER_THROTTLE_CYCLES          0x41
#define EVT_POWER_PCU_THROTTLING           0x42
#define EVT_POWER_SELF_REFRESH             0x43
#define EVT_POWER_CKE_CYCLES               0x83
#define EVT_POWER_CHANNEL_DLLOFF           0x84
#define EVT_POWER_CHANNEL_PD               0x85
#define EVT_POWER_CRITICAL_THROTTLE_CYCLES 0x86

#define UMASK_CAS_RD_REG       0x1
#define UMASK_CAS_RD_UNDERFILL 0x2
#define UMASK_CAS_RD           0x3
#define UMASK_CAS_WR_WMM       0x4
#define UMASK_CAS_WR_RMM       0x8
#define UMASK_CAS_WR           0xC
#define UMASK_CAS_ALL          0xF

/*******************************/
/* Missing from documentation? */
/*******************************/
#define UMASK_ACT_COUNT_RD   0x1
#define UMASK_ACT_COUNT_WR   0x2
#define UMASK_ACT_COUNT_BYP  0x8
#define UMASK_BYP_CMDS_ACT   0x1
#define UMASK_BYP_CMDS_CAS   0x2
#define UMASK_BYP_CMDS_PRE   0x4
/* ?? */

#define UMASK_PRE_PAGE_MISS  0x1
#define UMASK_PRE_PAGE_CLOSE 0x2

/*******************************/
/* Missing from documentation? */
/*******************************/
#define UMASK_PRE_RD  0x4
#define UMASK_PRE_WR  0x8
#define UMASK_PRE_BYP 0x16
