#ifndef MSR_CLOCKS_H
#define MSR_CLOCKS_H

void read_all_aperf(uint64_t *aperf); 
void read_all_mperf(uint64_t *mperf); 
void read_all_tsc  (uint64_t *tsc); 
void dump_clocks_terse();
void dump_clocks_terse_label();

#endif //MSR_CLOCKS_H
