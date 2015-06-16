
set(LIBMSR_HEADERS
  msr_clocks.h
  msr_counters.h
  msr_rapl.h
  msr_turbo.h
  signalCombined.h
  cpuid.h
  msr_core.h
  msr_misc.h
  msr_thermal.h
  profile.h)

install(FILES ${LIBMSR_HEADERS} DESTINATION include)
