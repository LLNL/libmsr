#ifndef MSR_CLOCKS_H
#define MSR_CLOCKS_H

#define NUM_PACKAGES 1

void read_aperf(int package, uint64_t *aperf); 
void read_mperf(int package, uint64_t *mperf); 
void read_tsc  (int package, uint64_t *tsc); 
double get_effective_frequency(int package);
void dump_clocks();
void dump_clocks_terse();
void dump_clocks_terse_label();

#endif //MSR_CLOCKS_H
