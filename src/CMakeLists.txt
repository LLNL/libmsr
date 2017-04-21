#
# Discover the target architecture and create the master header file
#
set(LIBMSR_TARGET_ARCH "" CACHE STRING "Force configuring for a particular architecture")
if (NOT LIBMSR_TARGET_ARCH)
    try_run(X86_ARCH_RUN_RESULT X86_ARCH_COMPILE_RESULT
            ${CMAKE_BINARY_DIR}/x86_arch ${CMAKE_SOURCE_DIR}/x86_arch.c
            COMPILE_OUTPUT_VARIABLE X86_ARCH_COMPILE_OUTPUT
            RUN_OUTPUT_VARIABLE LIBMSR_TARGET_ARCH)
    if(NOT X86_ARCH_COMPILE_RESULT)
        message(FATAL_ERROR "Failed to compile ${CMAKE_SOURCE_DIR}/x86_arch.c:\n${X86_ARCH_COMPILE_OUTPUT}")
    endif()
    if(NOT X86_ARCH_RUN_RESULT EQUAL 0)
        message(FATAL_ERROR "Bad return code running ${CMAKE_SOURCE_DIR}/x86_arch.c: ${X86_ARCH_RUN_RESULT}")
    endif()
endif()
if(NOT EXISTS ${CMAKE_SOURCE_DIR}/platform_headers/Intel${LIBMSR_TARGET_ARCH}.h)
    message(FATAL_ERROR
            "Unsupported model (no matching header found in platform_headers): ${LIBMSR_TARGET_ARCH}\n"
            "Set LIBMSR_TARGET_ARCH to a valid value to force a particular architecture.")
endif()
add_custom_command(OUTPUT ${CMAKE_SOURCE_DIR}/include/master.h
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/platform_headers/Intel${LIBMSR_TARGET_ARCH}.h ${CMAKE_SOURCE_DIR}/include/master.h
    DEPENDS ${CMAKE_SOURCE_DIR}/platform_headers/Intel${LIBMSR_TARGET_ARCH}.h
)

#
# Static and dynamic libs have the same sources, so make a variable.
#
set(LIBMSR_SOURCES
    cpuid.c
    csr_core.c
    csr_imc.c
    memhdlr.c
    libmsr_error.c
    msr_clocks.c
    msr_core.c
    msr_counters.c
    msr_misc.c
    msr_rapl.c
    msr_thermal.c
    msr_turbo.c
    # We specify this header so CMake will force its creation (see custom command above)
    ${CMAKE_SOURCE_DIR}/include/master.h
)

#
# Add dynamic library
#
add_library(msr SHARED ${LIBMSR_SOURCES})
target_link_libraries(msr m)

#
# Add static library with same base name as the dynamic lib.
#
add_library(msr-static STATIC ${LIBMSR_SOURCES})
target_link_libraries(msr-static m)
set_target_properties(msr-static PROPERTIES OUTPUT_NAME "msr")

#
# Install target should install the two library targets above.
#
install(TARGETS msr msr-static DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" EXPORT libmsr-libs)

#
# Headers are in ../include
#
include_directories(${PROJECT_SOURCE_DIR}/include)
