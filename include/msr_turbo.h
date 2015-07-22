#ifndef MSR_TURBO_H
#define MSR_TURBO_H

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

void enable_turbo();
void disable_turbo();
void dump_turbo(FILE * writeFile);

#ifdef __cplusplus
}
#endif

#endif //MSR_TURBO_H


