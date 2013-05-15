#ifndef MSR_CLOCKS_H
#define MSR_CLOCKS_H

#ifdef __cplusplus 
extern "C" {
#endif

void read_aperf(const int socket, uint64_t *aperf); 
void read_mperf(const int socket, uint64_t *mperf); 
void read_tsc  (const int socket, uint64_t *tsc); 
double get_effective_frequency(const int socket);
void dump_clocks(const int socket);

#ifdef __cplusplus 
}
#endif

#endif //MSR_CLOCKS_H
