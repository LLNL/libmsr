/* msr_common.h
 *
 * Data structures used across multiple modules.
 */
#include <stdint.h>
#ifndef MSR_COMMON_H
#define MSR_COMMON_H
extern int msr_debug;

#define MASK_RANGE(m,n) ((((uint64_t)1<<((m)-(n)+1))-1)<<(n))	// m>=n
#define MASK_VAL(x,m,n) (((uint64_t)(x)&MASK_RANGE((m),(n)))>>(n))

#define UNIT_SCALE(x,y) ((x)/(double)(1<<(y)))
#define UNIT_DESCALE(x,y) ((x)*(double)(1<<(y)))

#endif  // MSR_COMMON_H


