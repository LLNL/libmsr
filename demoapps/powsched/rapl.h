
/// Sets allocation values via the MSRs
void setAllocations(const double aw0, const double aw1);

/// Reads and packs the MSR values for send
int getReadings(char* buf, int buflen);

int rapl_init();
