#ifndef MSR_PROFILE_H
#define MSR_PROFILE_H

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include "msr_core.h"
#include "msr_thermal.h"
#include "msr_rapl.h"
#include "msr_clocks.h"

#ifdef __cplusplus
extern "C" {
#endif

void msr_profile();

#ifdef __cplusplus
}
#endif

#endif // MSR_PROFILE_H
