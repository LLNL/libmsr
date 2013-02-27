#ifndef MSR_CLOCKS_H
#define MSR_CLOCKS_H

void read_aperf(int package, uint64_t *aperf); 
void read_mperf(int package, uint64_t *mperf); 
void read_tsc  (int package, uint64_t *tsc); 
//double get_effective_frequency();
//TP

double get_effective_frequency(int package);

void dump_clocks();

#endif //MSR_CLOCKS_H
