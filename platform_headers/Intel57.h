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
#define COMPILED_ARCH 0x57   // Knights Landing

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
#define IA32_PERF_GLOBAL_STATUS   0x38E // Overflow condition can be found here
#define IA32_PERF_GLOBAL_CTRL     0x38F // Enables for fixed ctr0,1,and2 here
#define IA32_PERF_GLOBAL_OVF_CTRL 0x390 // Can clear the overflow here
#define IA32_FIXED_CTR0           0x309 // (R/W) Counts Instr_Retired.Any
#define IA32_FIXED_CTR1           0x30A // (R/W) Counts CPU_CLK_Unhalted.Core
#define IA32_FIXED_CTR2           0x30B // (R/W) Counts CPU_CLK_Unhalted.Ref

#define IA32_PMC0 0xC1
#define IA32_PMC1 0xC2

#define IA32_PERFEVTSEL0 0x186
#define IA32_PERFEVTSEL1 0x187

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

#define MSR_C1_PMON_BOX_CTL   0xE0C
#define MSR_C1_PMON_EVNTSEL0  0xE0D
#define MSR_C1_PMON_EVNTSEL1  0xE0E
#define MSR_C1_PMON_EVNTSEL2  0xE0F
#define MSR_C1_PMON_EVNTSEL3  0xE10
#define MSR_C1_BOX_FILTER     0xE11
#define MSR_C1_BOX_FILTER1    0xE12
#define MSR_C1_BOX_STATUS     0xE13
#define MSR_C1_PMON_CTR0      0xE14
#define MSR_C1_PMON_CTR1      0xE15
#define MSR_C1_PMON_CTR2      0xE16
#define MSR_C1_PMON_CTR3      0xE17

#define MSR_C2_PMON_BOX_CTL   0xE18
#define MSR_C2_PMON_EVNTSEL0  0xE19
#define MSR_C2_PMON_EVNTSEL1  0xE1A
#define MSR_C2_PMON_EVNTSEL2  0xE1B
#define MSR_C2_PMON_EVNTSEL3  0xE1C
#define MSR_C2_BOX_FILTER     0xE1D
#define MSR_C2_BOX_FILTER1    0xE1E
#define MSR_C2_BOX_STATUS     0xE1F
#define MSR_C2_PMON_CTR0      0xE20
#define MSR_C2_PMON_CTR1      0xE21
#define MSR_C2_PMON_CTR2      0xE22
#define MSR_C2_PMON_CTR3      0xE23

#define MSR_C3_PMON_BOX_CTL   0xE24
#define MSR_C3_PMON_EVNTSEL0  0xE25
#define MSR_C3_PMON_EVNTSEL1  0xE26
#define MSR_C3_PMON_EVNTSEL2  0xE27
#define MSR_C3_PMON_EVNTSEL3  0xE28
#define MSR_C3_BOX_FILTER     0xE29
#define MSR_C3_BOX_FILTER1    0xE2A
#define MSR_C3_BOX_STATUS     0xE2B
#define MSR_C3_PMON_CTR0      0xE2C
#define MSR_C3_PMON_CTR1      0xE2D
#define MSR_C3_PMON_CTR2      0xE2E
#define MSR_C3_PMON_CTR3      0xE2F

#define MSR_C4_PMON_BOX_CTL   0xE30
#define MSR_C4_PMON_EVNTSEL0  0xE31
#define MSR_C4_PMON_EVNTSEL1  0xE32
#define MSR_C4_PMON_EVNTSEL2  0xE33
#define MSR_C4_PMON_EVNTSEL3  0xE34
#define MSR_C4_BOX_FILTER     0xE35
#define MSR_C4_BOX_FILTER1    0xE36
#define MSR_C4_BOX_STATU      0xE37
#define MSR_C4_PMON_CTR0      0xE38
#define MSR_C4_PMON_CTR1      0xE39
#define MSR_C4_PMON_CTR2      0xE3A
#define MSR_C4_PMON_CTR3      0xE3B

#define MSR_C5_PMON_BOX_CTL   0xE3C
#define MSR_C5_PMON_EVNTSEL0  0xE3D
#define MSR_C5_PMON_EVNTSEL1  0xE3E
#define MSR_C5_PMON_EVNTSEL2  0xE3F
#define MSR_C5_PMON_EVNTSEL3  0xE40
#define MSR_C5_BOX_FILTER     0xE41
#define MSR_C5_BOX_FILTER1    0xE42
#define MSR_C5_BOX_STATU      0xE43
#define MSR_C5_PMON_CTR0      0xE44
#define MSR_C5_PMON_CTR1      0xE45
#define MSR_C5_PMON_CTR2      0xE46
#define MSR_C5_PMON_CTR3      0xE47

#define MSR_C6_PMON_BOX_CTL   0xE48
#define MSR_C6_PMON_EVNTSEL0  0xE49
#define MSR_C6_PMON_EVNTSEL1  0xE4A
#define MSR_C6_PMON_EVNTSEL2  0xE4B
#define MSR_C6_PMON_EVNTSEL3  0xE4C
#define MSR_C6_BOX_FILTER     0xE4D
#define MSR_C6_BOX_FILTER1    0xE4E
#define MSR_C6_BOX_STATU      0xE4F
#define MSR_C6_PMON_CTR0      0xE50
#define MSR_C6_PMON_CTR1      0xE51
#define MSR_C6_PMON_CTR2      0xE52
#define MSR_C6_PMON_CTR3      0xE53

#define MSR_C7_PMON_BOX_CTL   0xE54
#define MSR_C7_PMON_EVNTSEL0  0xE55
#define MSR_C7_PMON_EVNTSEL1  0xE56
#define MSR_C7_PMON_EVNTSEL2  0xE57
#define MSR_C7_PMON_EVNTSEL3  0xE58
#define MSR_C7_BOX_FILTER     0xE59
#define MSR_C7_BOX_FILTER1    0xE5A
#define MSR_C7_BOX_STATU      0xE5B
#define MSR_C7_PMON_CTR0      0xE5C
#define MSR_C7_PMON_CTR1      0xE5D
#define MSR_C7_PMON_CTR2      0xE5E
#define MSR_C7_PMON_CTR3      0xE5F

#define MSR_C8_PMON_BOX_CTL   0xE60
#define MSR_C8_PMON_EVNTSEL0  0xE61
#define MSR_C8_PMON_EVNTSEL1  0xE62
#define MSR_C8_PMON_EVNTSEL2  0xE63
#define MSR_C8_PMON_EVNTSEL3  0xE64
#define MSR_C8_BOX_FILTER     0xE65
#define MSR_C8_BOX_FILTER1    0xE66
#define MSR_C8_BOX_STATU      0xE67
#define MSR_C8_PMON_CTR0      0xE68
#define MSR_C8_PMON_CTR1      0xE69
#define MSR_C8_PMON_CTR2      0xE6A
#define MSR_C8_PMON_CTR3      0xE6B

#define MSR_C9_PMON_BOX_CTL   0xE6C
#define MSR_C9_PMON_EVNTSEL0  0xE6D
#define MSR_C9_PMON_EVNTSEL1  0xE6E
#define MSR_C9_PMON_EVNTSEL2  0xE6F
#define MSR_C9_PMON_EVNTSEL3  0xE70
#define MSR_C9_BOX_FILTER     0xE71
#define MSR_C9_BOX_FILTER1    0xE72
#define MSR_C9_BOX_STATU      0xE73
#define MSR_C9_PMON_CTR0      0xE74
#define MSR_C9_PMON_CTR1      0xE75
#define MSR_C9_PMON_CTR2      0xE76
#define MSR_C9_PMON_CTR3      0xE77

#define MSR_C10_PMON_BOX_CTL  0xE78
#define MSR_C10_PMON_EVNTSEL0 0xE79
#define MSR_C10_PMON_EVNTSEL1 0xE7A
#define MSR_C10_PMON_EVNTSEL2 0xE7B
#define MSR_C10_PMON_EVNTSEL3 0xE7C
#define MSR_C10_BOX_FILTER    0xE7D
#define MSR_C10_BOX_FILTER1   0xE7E
#define MSR_C10_BOX_STATU     0xE7F
#define MSR_C10_PMON_CTR0     0xE80
#define MSR_C10_PMON_CTR1     0xE81
#define MSR_C10_PMON_CTR2     0xE82
#define MSR_C10_PMON_CTR3     0xE83

#define MSR_C11_PMON_BOX_CTL  0xE84
#define MSR_C11_PMON_EVNTSEL0 0xE85
#define MSR_C11_PMON_EVNTSEL1 0xE86
#define MSR_C11_PMON_EVNTSEL2 0xE87
#define MSR_C11_PMON_EVNTSEL3 0xE88
#define MSR_C11_BOX_FILTER    0xE89
#define MSR_C11_BOX_FILTER1   0xE8A
#define MSR_C11_BOX_STATUS    0xE8B
#define MSR_C11_PMON_CTR0     0xE8C
#define MSR_C11_PMON_CTR1     0xE8D
#define MSR_C11_PMON_CTR2     0xE8E
#define MSR_C11_PMON_CTR3     0xE8F

#define MSR_C12_PMON_BOX_CTL  0xE90
#define MSR_C12_PMON_EVNTSEL0 0xE91
#define MSR_C12_PMON_EVNTSEL1 0xE92
#define MSR_C12_PMON_EVNTSEL2 0xE93
#define MSR_C12_PMON_EVNTSEL3 0xE94
#define MSR_C12_BOX_FILTER    0xE95
#define MSR_C12_BOX_FILTER1   0xE96
#define MSR_C12_BOX_STATUS    0xE97
#define MSR_C12_PMON_CTR0     0xE98
#define MSR_C12_PMON_CTR1     0xE99
#define MSR_C12_PMON_CTR2     0xE9A
#define MSR_C12_PMON_CTR3     0xE9B

#define MSR_C13_PMON_BOX_CTL  0xE9C
#define MSR_C13_PMON_EVNTSEL0 0xE9D
#define MSR_C13_PMON_EVNTSEL1 0xE9E
#define MSR_C13_PMON_EVNTSEL2 0xE9F
#define MSR_C13_PMON_EVNTSEL3 0xEA0
#define MSR_C13_BOX_FILTER    0xEA1
#define MSR_C13_BOX_FILTER1   0xEA2
#define MSR_C13_BOX_STATUS    0xEA3
#define MSR_C13_PMON_CTR0     0xEA4
#define MSR_C13_PMON_CTR1     0xEA5
#define MSR_C13_PMON_CTR2     0xEA6
#define MSR_C13_PMON_CTR3     0xEA7

#define MSR_C14_PMON_BOX_CTL  0xEA8
#define MSR_C14_PMON_EVNTSEL0 0xEA9
#define MSR_C14_PMON_EVNTSEL1 0xEAA
#define MSR_C14_PMON_EVNTSEL2 0xEAB
#define MSR_C14_PMON_EVNTSEL3 0xEAC
#define MSR_C14_BOX_FILTER    0xEAD
#define MSR_C14_BOX_FILTER1   0xEAE
#define MSR_C14_BOX_STATUS    0xEAF
#define MSR_C14_PMON_CTR0     0xEB0
#define MSR_C14_PMON_CTR1     0xEB1
#define MSR_C14_PMON_CTR2     0xEB2
#define MSR_C14_PMON_CTR3     0xEB3

#define MSR_C15_PMON_BOX_CTL  0xEB4
#define MSR_C15_PMON_EVNTSEL0 0xEB5
#define MSR_C15_PMON_EVNTSEL1 0xEB6
#define MSR_C15_PMON_EVNTSEL2 0xEB7
#define MSR_C15_PMON_EVNTSEL3 0xEB8
#define MSR_C15_BOX_FILTER    0xEB9
#define MSR_C15_BOX_FILTER1   0xEBA
#define MSR_C15_BOX_STATUS    0xEBB
#define MSR_C15_PMON_CTR0     0xEBC
#define MSR_C15_PMON_CTR1     0xEBD
#define MSR_C15_PMON_CTR2     0xEBE
#define MSR_C15_PMON_CTR3     0xEBF

#define MSR_C16_PMON_BOX_CTL  0xEC0
#define MSR_C16_PMON_EVNTSEL0 0xEC1
#define MSR_C16_PMON_EVNTSEL1 0xEC2
#define MSR_C16_PMON_EVNTSEL2 0xEC3
#define MSR_C16_PMON_EVNTSEL3 0xEC4
#define MSR_C16_BOX_FILTER    0xEC5
#define MSR_C16_BOX_FILTER1   0xEC6
#define MSR_C16_BOX_STATUS    0xEC7
#define MSR_C16_PMON_CTR0     0xEC8
#define MSR_C16_PMON_CTR1     0xEC9
#define MSR_C16_PMON_CTR2     0xECA
#define MSR_C16_PMON_CTR3     0xECB

#define MSR_C17_PMON_BOX_CTL  0xECC
#define MSR_C17_PMON_EVNTSEL0 0xECD
#define MSR_C17_PMON_EVNTSEL1 0xECE
#define MSR_C17_PMON_EVNTSEL2 0xECF
#define MSR_C17_PMON_EVNTSEL3 0xED0
#define MSR_C17_BOX_FILTER    0xED1
#define MSR_C17_BOX_FILTER1   0xED2
#define MSR_C17_BOX_STATUS    0xED3
#define MSR_C17_PMON_CTR0     0xED4
#define MSR_C17_PMON_CTR1     0xED5
#define MSR_C17_PMON_CTR2     0xED6
#define MSR_C17_PMON_CTR3     0xED7

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
#define IA32_THERM_INTERRUPT         0x19B
#define IA32_THERM_STATUS            0x19C
#define MSR_THERM2_CTL               0x19D
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
