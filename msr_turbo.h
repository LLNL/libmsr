#ifndef MSR_TURBO_H
#define MSR_TURBO_H

#ifdef __cplusplus 
extern "C" {
#endif

void enable_turbo( int cpu );
void disable_turbo( int cpu );
void enable_all_turbo( );
void disable_all_turbo( );
void dump_turbo( );

#ifdef __cplusplus 
}
#endif

#endif //MSR_TURBO_H


