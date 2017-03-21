set(LIBMSR_HEADERS
    cpuid.h
    csr_core.h
    csr_imc.h
    libmsr_error.h
    master.h
    memhdlr.h
    msr_clocks.h
    msr_core.h
    msr_counters.h
    msr_misc.h
    msr_rapl.h
    msr_thermal.h
    msr_turbo.h
)

install(FILES ${LIBMSR_HEADERS} DESTINATION
    "${CMAKE_INSTALL_PREFIX}/include"
)
