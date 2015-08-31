
/// Initialize the library to read from the rapl MSRs
void read_rapl_init();

/// Read the current consumption and alloction
// ret must be an array of length 8
// idx 0,1 report consumption in joules for sockets 0 and 1
// idx 2,3 report allocation in joules for sockets 0 and 1
// idx 4,5 report consumption in watts for sockets 0 and 1
// idx 6,7 report allocation in joules for sockets 0 and 1
void read_rapl_energy_and_power(double* ret);
