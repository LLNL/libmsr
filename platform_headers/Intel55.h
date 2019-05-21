/* Intel Skylake  Master Header File
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
 * FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for
 * more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with libmsr. If not, see <http://www.gnu.org/licenses/>.
 *
 * This material is based upon work supported by the U.S. Department of
 * Energy's Lawrence Livermore National Laboratory. Office of Science, under
 * Award number DE-AC52-07NA27344.
 *
 */

#define COMPILED_VEND 0x8086 // Intel
#define COMPILED_ARCH 0x55   // Skylake

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
#define IA32_PERF_GLOBAL_STATUS   0x38E // Overflow condition can be found here
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

#define MSR_PCU_PMON_EVNTSEL0 0x711
#define MSR_PCU_PMON_EVNTSEL1 0x712
#define MSR_PCU_PMON_EVNTSEL2 0x713
#define MSR_PCU_PMON_EVNTSEL3 0x714
#define MSR_PCU_PMON_CTR0     0x717
#define MSR_PCU_PMON_CTR1     0x718
#define MSR_PCU_PMON_CTR2     0x719
#define MSR_PCU_PMON_CTR3     0x71A

#define MSR_C0_PMON_BOX_CTL   0xE00
#define MSR_C0_PMON_EVNTSEL0  0xE01
#define MSR_C0_PMON_EVNTSEL1  0xE02
#define MSR_C0_PMON_EVNTSEL2  0xE03
#define MSR_C0_PMON_EVNTSEL3  0xE04
#define MSR_C0_BOX_FILTER     0xE05
#define MSR_C0_BOX_FILTER1    0xE06
#define MSR_C0_BOX_STATUS     0xE07
#define MSR_C0_PMON_CTR0      0xE08
#define MSR_C0_PMON_CTR1      0xE09
#define MSR_C0_PMON_CTR2      0xE0A
#define MSR_C0_PMON_CTR3      0xE0B

#define MSR_C1_PMON_BOX_CTL   0xE10
#define MSR_C1_PMON_EVNTSEL0  0xE11
#define MSR_C1_PMON_EVNTSEL1  0xE12
#define MSR_C1_PMON_EVNTSEL2  0xE13
#define MSR_C1_PMON_EVNTSEL3  0xE14
#define MSR_C1_BOX_FILTER     0xE15
#define MSR_C1_BOX_FILTER1    0xE16
#define MSR_C1_BOX_STATUS     0xE17
#define MSR_C1_PMON_CTR0      0xE18
#define MSR_C1_PMON_CTR1      0xE19
#define MSR_C1_PMON_CTR2      0xE1A
#define MSR_C1_PMON_CTR3      0xE1B

#define MSR_C2_PMON_BOX_CTL   0xE20
#define MSR_C2_PMON_EVNTSEL0  0xE21
#define MSR_C2_PMON_EVNTSEL1  0xE22
#define MSR_C2_PMON_EVNTSEL2  0xE23
#define MSR_C2_PMON_EVNTSEL3  0xE24
#define MSR_C2_BOX_FILTER     0xE25
#define MSR_C2_BOX_FILTER1    0xE26
#define MSR_C2_BOX_STATUS     0xE27
#define MSR_C2_PMON_CTR0      0xE28
#define MSR_C2_PMON_CTR1      0xE29
#define MSR_C2_PMON_CTR2      0xE2A
#define MSR_C2_PMON_CTR3      0xE2B

#define MSR_C3_PMON_BOX_CTL   0xE30
#define MSR_C3_PMON_EVNTSEL0  0xE31
#define MSR_C3_PMON_EVNTSEL1  0xE32
#define MSR_C3_PMON_EVNTSEL2  0xE33
#define MSR_C3_PMON_EVNTSEL3  0xE34
#define MSR_C3_BOX_FILTER     0xE35
#define MSR_C3_BOX_FILTER1    0xE36
#define MSR_C3_BOX_STATUS     0xE37
#define MSR_C3_PMON_CTR0      0xE38
#define MSR_C3_PMON_CTR1      0xE39
#define MSR_C3_PMON_CTR2      0xE3A
#define MSR_C3_PMON_CTR3      0xE3B

#define MSR_C4_PMON_BOX_CTL   0xE40
#define MSR_C4_PMON_EVNTSEL0  0xE41
#define MSR_C4_PMON_EVNTSEL1  0xE42
#define MSR_C4_PMON_EVNTSEL2  0xE43
#define MSR_C4_PMON_EVNTSEL3  0xE44
#define MSR_C4_BOX_FILTER     0xE45
#define MSR_C4_BOX_FILTER1    0xE46
#define MSR_C4_BOX_STATU      0xE47
#define MSR_C4_PMON_CTR0      0xE48
#define MSR_C4_PMON_CTR1      0xE49
#define MSR_C4_PMON_CTR2      0xE4A
#define MSR_C4_PMON_CTR3      0xE4B

#define MSR_C5_PMON_BOX_CTL   0xE50
#define MSR_C5_PMON_EVNTSEL0  0xE51
#define MSR_C5_PMON_EVNTSEL1  0xE52
#define MSR_C5_PMON_EVNTSEL2  0xE53
#define MSR_C5_PMON_EVNTSEL3  0xE54
#define MSR_C5_BOX_FILTER     0xE55
#define MSR_C5_BOX_FILTER1    0xE56
#define MSR_C5_BOX_STATU      0xE57
#define MSR_C5_PMON_CTR0      0xE58
#define MSR_C5_PMON_CTR1      0xE59
#define MSR_C5_PMON_CTR2      0xE5A
#define MSR_C5_PMON_CTR3      0xE5B

#define MSR_C6_PMON_BOX_CTL   0xE60
#define MSR_C6_PMON_EVNTSEL0  0xE61
#define MSR_C6_PMON_EVNTSEL1  0xE62
#define MSR_C6_PMON_EVNTSEL2  0xE63
#define MSR_C6_PMON_EVNTSEL3  0xE64
#define MSR_C6_BOX_FILTER     0xE65
#define MSR_C6_BOX_FILTER1    0xE66
#define MSR_C6_BOX_STATU      0xE67
#define MSR_C6_PMON_CTR0      0xE68
#define MSR_C6_PMON_CTR1      0xE69
#define MSR_C6_PMON_CTR2      0xE6A
#define MSR_C6_PMON_CTR3      0xE6B

#define MSR_C7_PMON_BOX_CTL   0xE70
#define MSR_C7_PMON_EVNTSEL0  0xE71
#define MSR_C7_PMON_EVNTSEL1  0xE72
#define MSR_C7_PMON_EVNTSEL2  0xE73
#define MSR_C7_PMON_EVNTSEL3  0xE74
#define MSR_C7_BOX_FILTER     0xE75
#define MSR_C7_BOX_FILTER1    0xE76
#define MSR_C7_BOX_STATU      0xE77
#define MSR_C7_PMON_CTR0      0xE78
#define MSR_C7_PMON_CTR1      0xE79
#define MSR_C7_PMON_CTR2      0xE7A
#define MSR_C7_PMON_CTR3      0xE7B

#define MSR_C8_PMON_BOX_CTL   0xE80
#define MSR_C8_PMON_EVNTSEL0  0xE81
#define MSR_C8_PMON_EVNTSEL1  0xE82
#define MSR_C8_PMON_EVNTSEL2  0xE83
#define MSR_C8_PMON_EVNTSEL3  0xE84
#define MSR_C8_BOX_FILTER     0xE85
#define MSR_C8_BOX_FILTER1    0xE86
#define MSR_C8_BOX_STATU      0xE87
#define MSR_C8_PMON_CTR0      0xE88
#define MSR_C8_PMON_CTR1      0xE89
#define MSR_C8_PMON_CTR2      0xE8A
#define MSR_C8_PMON_CTR3      0xE8B

#define MSR_C9_PMON_BOX_CTL   0xE90
#define MSR_C9_PMON_EVNTSEL0  0xE91
#define MSR_C9_PMON_EVNTSEL1  0xE92
#define MSR_C9_PMON_EVNTSEL2  0xE93
#define MSR_C9_PMON_EVNTSEL3  0xE94
#define MSR_C9_BOX_FILTER     0xE95
#define MSR_C9_BOX_FILTER1    0xE96
#define MSR_C9_BOX_STATU      0xE97
#define MSR_C9_PMON_CTR0      0xE98
#define MSR_C9_PMON_CTR1      0xE99
#define MSR_C9_PMON_CTR2      0xE9A
#define MSR_C9_PMON_CTR3      0xE9B

#define MSR_C10_PMON_BOX_CTL  0xEA0
#define MSR_C10_PMON_EVNTSEL0 0xEA1
#define MSR_C10_PMON_EVNTSEL1 0xEA2
#define MSR_C10_PMON_EVNTSEL2 0xEA3
#define MSR_C10_PMON_EVNTSEL3 0xEA4
#define MSR_C10_BOX_FILTER    0xEA5
#define MSR_C10_BOX_FILTER1   0xEA6
#define MSR_C10_BOX_STATU     0xEA7
#define MSR_C10_PMON_CTR0     0xEA8
#define MSR_C10_PMON_CTR1     0xEA9
#define MSR_C10_PMON_CTR2     0xEAA
#define MSR_C10_PMON_CTR3     0xEAB

#define MSR_C11_PMON_BOX_CTL  0xEB0
#define MSR_C11_PMON_EVNTSEL0 0xEB1
#define MSR_C11_PMON_EVNTSEL1 0xEB2
#define MSR_C11_PMON_EVNTSEL2 0xEB3
#define MSR_C11_PMON_EVNTSEL3 0xEB4
#define MSR_C11_BOX_FILTER    0xEB5
#define MSR_C11_BOX_FILTER1   0xEB6
#define MSR_C11_BOX_STATUS    0xEB7
#define MSR_C11_PMON_CTR0     0xEB8
#define MSR_C11_PMON_CTR1     0xEB9
#define MSR_C11_PMON_CTR2     0xEBA
#define MSR_C11_PMON_CTR3     0xEBB

#define MSR_C12_PMON_BOX_CTL  0xEC0
#define MSR_C12_PMON_EVNTSEL0 0xEC1
#define MSR_C12_PMON_EVNTSEL1 0xEC2
#define MSR_C12_PMON_EVNTSEL2 0xEC3
#define MSR_C12_PMON_EVNTSEL3 0xEC4
#define MSR_C12_BOX_FILTER    0xEC5
#define MSR_C12_BOX_FILTER1   0xEC6
#define MSR_C12_BOX_STATUS    0xEC7
#define MSR_C12_PMON_CTR0     0xEC8
#define MSR_C12_PMON_CTR1     0xEC9
#define MSR_C12_PMON_CTR2     0xECA
#define MSR_C12_PMON_CTR3     0xECB

#define MSR_C13_PMON_BOX_CTL  0xED0
#define MSR_C13_PMON_EVNTSEL0 0xED1
#define MSR_C13_PMON_EVNTSEL1 0xED2
#define MSR_C13_PMON_EVNTSEL2 0xED3
#define MSR_C13_PMON_EVNTSEL3 0xED4
#define MSR_C13_BOX_FILTER    0xED5
#define MSR_C13_BOX_FILTER1   0xED6
#define MSR_C13_BOX_STATUS    0xED7
#define MSR_C13_PMON_CTR0     0xED8
#define MSR_C13_PMON_CTR1     0xED9
#define MSR_C13_PMON_CTR2     0xEDA
#define MSR_C13_PMON_CTR3     0xEDB

#define MSR_C14_PMON_BOX_CTL  0xEE0
#define MSR_C14_PMON_EVNTSEL0 0xEE1
#define MSR_C14_PMON_EVNTSEL1 0xEE2
#define MSR_C14_PMON_EVNTSEL2 0xEE3
#define MSR_C14_PMON_EVNTSEL3 0xEE4
#define MSR_C14_BOX_FILTER    0xEE5
#define MSR_C14_BOX_FILTER1   0xEE6
#define MSR_C14_BOX_STATUS    0xEE7
#define MSR_C14_PMON_CTR0     0xEE8
#define MSR_C14_PMON_CTR1     0xEE9
#define MSR_C14_PMON_CTR2     0xEEA
#define MSR_C14_PMON_CTR3     0xEEB

#define MSR_C15_PMON_BOX_CTL  0xEF0
#define MSR_C15_PMON_EVNTSEL0 0xEF1
#define MSR_C15_PMON_EVNTSEL1 0xEF2
#define MSR_C15_PMON_EVNTSEL2 0xEF3
#define MSR_C15_PMON_EVNTSEL3 0xEF4
#define MSR_C15_BOX_FILTER    0xEF5
#define MSR_C15_BOX_FILTER1   0xEF6
#define MSR_C15_BOX_STATUS    0xEF7
#define MSR_C15_PMON_CTR0     0xEF8
#define MSR_C15_PMON_CTR1     0xEF9
#define MSR_C15_PMON_CTR2     0xEFA
#define MSR_C15_PMON_CTR3     0xEFB

#define MSR_C16_PMON_BOX_CTL  0xF00
#define MSR_C16_PMON_EVNTSEL0 0xF01
#define MSR_C16_PMON_EVNTSEL1 0xF02
#define MSR_C16_PMON_EVNTSEL2 0xF03
#define MSR_C16_PMON_EVNTSEL3 0xF04
#define MSR_C16_BOX_FILTER    0xF05
#define MSR_C16_BOX_FILTER1   0xF06
#define MSR_C16_BOX_STATUS    0xF07
#define MSR_C16_PMON_CTR0     0xF08
#define MSR_C16_PMON_CTR1     0xF09
#define MSR_C16_PMON_CTR2     0xF0A
#define MSR_C16_PMON_CTR3     0xF0B

#define MSR_C17_PMON_BOX_CTL  0xF10
#define MSR_C17_PMON_EVNTSEL0 0xF11
#define MSR_C17_PMON_EVNTSEL1 0xF12
#define MSR_C17_PMON_EVNTSEL2 0xF13
#define MSR_C17_PMON_EVNTSEL3 0xF14
#define MSR_C17_BOX_FILTER    0xF15
#define MSR_C17_BOX_FILTER1   0xF16
#define MSR_C17_BOX_STATUS    0xF17
#define MSR_C17_PMON_CTR0     0xF18
#define MSR_C17_PMON_CTR1     0xF19
#define MSR_C17_PMON_CTR2     0xF1A
#define MSR_C17_PMON_CTR3     0xF1B

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
#define MSR_PKG_ENERGY_STATUS  0x611 // ro sic;
#define MSR_PKG_POWER_INFO     0x614 // rw text states ro

#define MSR_PKG_PERF_STATUS    0x613 // ro
#define MSR_DRAM_POWER_LIMIT   0x618 // rw
#define MSR_DRAM_ENERGY_STATUS 0x619 // ro. sic;
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
#define IA32_MISC_ENABLE           0x1A0
#define IA32_PERF_CTL              0x199
#define MSR_TURBO_ACTIVATION_RATIO 0x64C
#define MSR_TURBO_RATIO_LIMIT      0x1AD
#define MSR_TURBO_RATIO_LIMIT1     0x1AE

/***********/
/* CSR iMC */
/***********/
#define IMC0_DEV         20
#define IMC0_2_DEV       (IMC0_DEV + 1)
#define IMC1_DEV         23
#define IMC1_2_DEV       (IMC1_DEV + 1)
#define IMC_CH0_FUNC     0
#define IMC_CH1_FUNC     1
#define IMC_CH2_FUNC     0
#define IMC_CH3_FUNC     1

#define CSR_PMONCTRCFG0  0xD8
#define CSR_PMONCTRCFG1  0xDC
#define CSR_PMONCTRCFG2  0xE0
#define CSR_PMONCTRCFG3  0xE8

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
#define EVT_VMSE_WR_PUSH                   0x90
#define EVT_VMSE_MXB_WR_OCCUPANCY          0x91
#define EVT_RD_CAS_PRIO                    0xA0
#define EVT_BYP_CMDS                       0xA1
#define EVT_RD_CAS_RANK0                   0xB0
#define EVT_RD_CAS_RANK1                   0xB1
#define EVT_RD_CAS_RANK2                   0xB2
#define EVT_RD_CAS_RANK3                   0xB3
#define EVT_RD_CAS_RANK4                   0xB4
#define EVT_RD_CAS_RANK5                   0xB5
#define EVT_RD_CAS_RANK6                   0xB6
#define EVT_RD_CAS_RANK7                   0xB7
#define EVT_WR_CAS_RANK0                   0xB8
#define EVT_WR_CAS_RANK1                   0xB9
#define EVT_WR_CAS_RANK2                   0xBA
#define EVT_WR_CAS_RANK3                   0xBB
#define EVT_WR_CAS_RANK4                   0xBC
#define EVT_WR_CAS_RANK5                   0xBD
#define EVT_WR_CAS_RANK6                   0xBE
#define EVT_WR_CAS_RANK7                   0xBF
#define EVT_WMM_TO_RMM                     0xC0
#define EVT_WRONG_MM                       0xC1

#define UMASK_CAS_RD_REG       0x1
#define UMASK_CAS_RD_UNDERFILL 0x2
#define UMASK_CAS_RD           0x3
#define UMASK_CAS_WR_WMM       0x4
#define UMASK_CAS_WR_RMM       0x8
#define UMASK_CAS_WR           0xC
#define UMASK_CAS_ALL          0xF
#define UMASK_CAS_RD_WMM       0x16
#define UMASK_CAS_RD_RMM       0x32

#define UMASK_ACT_COUNT_RD  0x1
#define UMASK_ACT_COUNT_WR  0x2
#define UMASK_ACT_COUNT_BYP 0x8

#define UMASK_BYP_CMDS_ACT  0x1
#define UMASK_BYP_CMDS_CAS  0x2
#define UMASK_BYP_CMDS_PRE  0x4
// ... there are lots more of these...

#define UMASK_PRE_PAGE_MISS  0x1
#define UMASK_PRE_PAGE_CLOSE 0x2
#define UMASK_PRE_RD         0x4
#define UMASK_PRE_WR         0x8
#define UMASK_PRE_BYP        0x16
