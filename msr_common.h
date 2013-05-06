/* msr_common.h
 *
 * Macros common to several modules.
 */
#include <stdint.h>
#ifndef MSR_COMMON_H
#define MSR_COMMON_H
extern int msr_debug;

/* MASK_RANGE
 * Create a mask from bit m to n. 
 * 63 >= m >= n >= 0
 * Example:  MASK_RANGE(4,2) -->     (((1<<((4)-(2)+1))-1)<<(2)) 
 * 				     (((1<<          3)-1)<<(2))
 * 				     ((               4-1)<<(2))
 * 				     (                  3)<<(2))
 * 				     (                       24) = b11000
 */ 				 
#define MASK_RANGE(m,n) ((((uint64_t)1<<((m)-(n)+1))-1)<<(n))	// m>=n

/* MASK_VAL
 * Return the value of x after applying bitmask (m,n).
 * 63 >= m >= n >= 0
 * Example:  MASK_RANGE(17,4,2) --> 17&24 = b10001 & b11000 = b10000
 */
#define MASK_VAL(x,m,n) (((uint64_t)(x)&MASK_RANGE((m),(n)))>>(n))

/* UNIT_SCALE 
 * Calculates x/(2^y).  
 * Example:  The RAPL interface measures power in units of 1/(2^p) watts,
 * where is expressed as bits 0:3 of the RAPL_POWER_UNIT MSR, with the
 * default value y=b011 (thus 1/8th of a Watt).  Reading a power values
 * of x=b10000 translates into UNIT_SCALE(x,y) Watts, and a power cap of 
 * x Watts translates to a representation of UNIT_DESCALE(x,y) units.
 *
 */
#define UNIT_SCALE(x,y) ((x)/(double)(1<<(y)))
#define UNIT_DESCALE(x,y) ((x)*(double)(1<<(y)))

#endif  // MSR_COMMON_H


