/* checkCPUID.c
 *
 * Author: Kathleen Shoga
 * Edited by: Scott Walker
 * Initial cpuid function based off of an example on wikipedia:
 * http://en.wikipedia.org/wiki/CPUID#rax.3D0:_Get_vendor_ID
 *
 */
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
// Two defines below from Barry Rountree
#define MASK_RANGE(m,n) ((((uint32_t)1<<((m)-(n)+1))-1)<<(n))
#define MASK_VAL(x,m,n) (((uint32_t)(x)&MASK_RANGE((m),(n)))>>(n))


void cpuid(uint32_t leaf, uint32_t *rax, uint32_t *rbx, uint32_t *rcx, uint32_t *rdx)
{
	asm volatile(
		"\txchg %%rbx, %%edi\n"
        "\tcpuid\n"
        "\txchg %%rbx, %%edi"
	        	:"=a" (*rax), "=D" (*rbx), "=c" (*rcx), "=d" (*rdx)
	        	:"a" (leaf)
		);
}

void cpuidInput_rax_rcx(uint32_t leafa, uint32_t leafc, uint32_t *rax, uint32_t *rbx, uint32_t *rcx, uint32_t *rdx)
{
	asm volatile(
			"xchg %%rbx, %%edi\n"
            "\tcpuid\n"
            "\txchg %%rbx, %%edi"
                :"=a" (*rax), "=D" (*rbx), "=c" (*rcx), "=d" (*rdx)
                : "a" (leafa), "c" (leafc)
		    );
}

// RAPL-------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

// Note: RAPL does not need to check cpuid specifically (Manual 3C and 3B) may be in another source

// CLOCKS/TURBO-----------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

// NOTE: PERF_CTL: does not need to check cpuid specifically (Manual 3C) may be in another source

bool cpuid_MPERF_and_APERF()
{
	uint32_t rax, rbx, rcx, rdx;
	int leaf = 6;
	cpuid(leaf, &rax, &rbx, &rcx, &rdx);
	if(MASK_VAL(rcx, 0, 0) == 1) {
		return true;
	}
	else {
		return false;
	}
}

bool cpuid_timeStampCounter_avail()
{
	uint32_t rax, rbx, rcx, rdx;
	int leaf = 1;
	cpuid(leaf, &rax, &rbx, &rcx, &rdx);
	if(MASK_VAL(rdx,4,4) == 1) {
		return true;
	}
	else {
		return false;
	}
}

// PEBS--------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------

// NOTE: DS_AREA: does not need to check cpuid specifically (Manual Vol 3C) may be in another source
// NOTE: PEBS_ENABLE: does not need to check cpuid specifically (Manual Vol 3C) may be in another source

//--------------------------------PMC(insert number here) function-----------------------------------

int cpuid_PMC_num()
{
	uint32_t rax, rbx, rcx, rdx;
	int leaf = 10; // 0A
	cpuid(leaf, &rax, &rbx, &rcx, &rdx);
	return MASK_VAL(rax, 15, 8);		// This value tells which PMC's are available
						// If it is > 3 then up to PMC3 is usable
						// ...
						// If	    > 0 then PMC0 is usable
						// If == 0, then none are usable
}

//------------------------------PERFEVTSEL (0x186, 0x187, 0x188, 0x189) function---------------------

int cpuid_PERFEVTSEL_num()
{
	uint32_t rax, rbx, rcx, rdx;
	int leaf = 10; // 0A
	cpuid(leaf, &rax, &rbx, &rcx, &rdx);
	return MASK_VAL(rax, 15,8);		// This value tells which PERFEVTSEL's are available
       						// If it is > 3 then up to PERFEVTSEL3 is usable
						// If	    > 2 then up to PERFEVTSEL2 is usable
						// ...
						// If	    > 0 then PERFEVTSEL0 is usable
						// If == 0, then none are usable	
}

//------------------------------------PERF_GLOBAL_CTRL (0x38f) functions-----------------------------

bool cpuid_perf_global_ctrl_EN_PMC()
{
	uint32_t rax, rbx, rcx, rdx;
	int leaf = 10; // 0A
	cpuid(leaf, &rax, &rbx, &rcx, &rdx);
	if(MASK_VAL(rax, 7,0) > 0) {
		return true;
	}
	else {
		return false;
	}	
}

bool cpuid_perf_global_ctrl_EN_FIXED_CTRnum()
{
	uint32_t rax, rbx, rcx, rdx;
	int leaf = 10; // 0A
	cpuid(leaf, &rax, &rbx, &rcx, &rdx);
	if(MASK_VAL(rax, 7,0) > 1) {
		return true;
	}
	else {
		return false;
	}	
}
// THERMAL---------------------------------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------------------------------

//-----------------------------------------MISC_ENABLE functions-------------------------------------

bool cpuid_misc_enable_turboBoost()
{
	uint32_t rax, rbx, rcx, rdx;
	int leaf = 6;
	cpuid(leaf, &rax, &rbx, &rcx, &rdx);
	if(MASK_VAL(rax, 1, 1) == 1) {
		return true;
	}
	else {
		return false;
	}
}

bool cpuid_misc_enable_xTPRmessageDisable()
{
	uint32_t rax, rbx, rcx, rdx;
	int leaf = 1;
	cpuid(leaf, &rax, &rbx, &rcx, &rdx);
	if(MASK_VAL(rcx, 14, 14) == 1) {
		return true;
	}
	else { 
		return false;
	}
}

bool cpuid_misc_enable_XDbitDisable()
{
	uint32_t rax, rbx, rcx, rdx;
	int leaf = 2147483649; // 80000001H
	cpuid(leaf, &rax, &rbx, &rcx, &rdx);
	if(MASK_VAL(rdx, 20, 20) == 1) {
		return true;
	}
	else {
		return false;
	}
}

//---------------------------------------CLOCK_MOD functions-----------------------------------------

bool cpuid_clock_mod_extended()
{
	uint32_t rax, rbx, rcx, rdx;
	int leaf = 6;
	cpuid(leaf, &rax, &rbx, &rcx, &rdx);
	if(MASK_VAL(rax, 5,5) == 1) {
		return true;
	}
	else {
		return false;
	}
}

//--------------------------------------THERMAL functions--------------------------------------------

bool cpuid_therm_stat_therm_thresh()
{
	uint32_t rax, rbx, rcx, rdx;
	int leaf = 1;
	cpuid(leaf, &rax, &rbx, &rcx, &rdx);
	if(MASK_VAL(rcx, 8, 8) == 1) {
		return true;
	}
	else {
		return false;
	}
}

bool cpuid_therm_stat_powerlimit()
{
	uint32_t rax, rbx, rcx, rdx;
	int leaf = 6;
	cpuid(leaf, &rax, &rbx, &rcx, &rdx);
	if(MASK_VAL(rax, 4, 4) == 1) {
		return true;
	}
	else {
		return false;
	}
}

bool cpuid_therm_stat_readout()
{
	uint32_t rax, rbx, rcx, rdx;
	int leaf = 6;
	cpuid(leaf, &rax, &rbx, &rcx, &rdx);
	if(MASK_VAL(rax, 0, 0) == 1) {
		return true;
	}
	else {
		return false;
	}
}

bool cpuid_therm_interrupt_powerlimit()
{
	uint32_t rax, rbx, rcx, rdx;
	int leaf = 6;
	cpuid(leaf, &rax, &rbx, &rcx, &rdx);
	if(MASK_VAL(rax, 4, 4) == 1) {
		return true;
	}
	else {
		return false;
	}
}

bool cpuid_pkg_therm_Stat_AND_Interrupt()
{
	uint32_t rax, rbx, rcx, rdx;
	int leaf = 6;
	cpuid(leaf, &rax, &rbx, &rcx, &rdx);
	if(MASK_VAL(rax, 6, 6) == 1) {
		return true;
	}
	else {
		return false;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////
//--------------------------------------General Machine Info-------------------------------------------

uint32_t cpuid_maxleaf()
{
	uint32_t rax, rbx, rcx, rdx;
	int leaf=0;
	cpuid(leaf, &rax, &rbx, &rcx, &rdx);
	return rax;
}

void cpuid_printVendorID()
{
	uint32_t rax, rbx, rcx, rdx;
	int leaf=0, i=0;
	cpuid(leaf, &rax, &rbx, &rcx, &rdx);
	for(i = 0; i < 32; i+=8)
	{
		printf("%c", MASK_VAL(rbx,7+i,0+i));
	}
	for(i = 0; i < 32; i+=8)
	{
		printf("%c", MASK_VAL(rdx,7+i,0+i));
	}
	for(i = 0; i < 32; i+=8)
	{
		printf("%c", MASK_VAL(rcx,7+i,0+i));
	}
	printf("\n");
}

// For the two functions below, see Manual Vol. 3A, Section 8.6 for details
// about what the numbers mean

int cpuid_pkg_maxPhysicalProcessorCores()
{
	uint32_t rax, rbx, rcx, rdx;
	int leafa = 4, leafc = 0;
	cpuidInput_rax_rcx(leafa, leafc, &rax, &rbx, &rcx, &rdx);
	return MASK_VAL(rax, 31,26) + 1;
}

int cpuid_pkg_maxLogicalProcessors()
{
	uint32_t rax, rbx, rcx, rdx;
	int leaf = 1;
	cpuid(leaf, &rax, &rbx, &rcx, &rdx);
	return MASK_VAL(rax, 23,16);
}

// The 2 functions below are for the fixed performance counters. See Manual Vol. 18.2.2.1 
int cpuid_num_fixed_perf_counters()
{
	uint32_t rax, rbx, rcx, rdx;
	int leaf= 10; //0A
	cpuid(leaf, &rax, &rbx, &rcx, &rdx);
	return MASK_VAL(rdx,4,0);
}

int cpuid_width_fixed_perf_counters()
{
	uint32_t rax, rbx, rcx, rdx;
	int leaf= 10; //0A
	cpuid(leaf, &rax, &rbx, &rcx, &rdx);
	return MASK_VAL(rdx,12,5);
}
/*
int main()
{
	printf("\n");
	cpuid_printVendorID();
//- This block checks for processor info----------------------
	int x = cpuid_pkg_maxPhysicalProcessorCores();
	printf("%d\n", x);

	x = cpuid_pkg_maxLogicalProcessors();
	printf("%d\n", x);
//------------------------------------------------------------

//- This block (below) checks for the fixed number of performance counters etc.--------
	int x = cpuid_num_fixed_perf_counters();
	printf("Number of fixed performance counters\n%d\n",x);

	x = cpuid_width_fixed_perf_counters();
	printf("Width of fixed performance counters\n%d\n",x);
//-------------------------------------------------------------------------------------

	return 0;
}
*/
