# Static and dynamic libs have the same sources, so make a variable.
set(LIBMSR_SOURCES
  msr_clocks.c
  msr_core.c
  msr_counters.c
  msr_misc.c
  msr_rapl.c
  msr_thermal.c
  msr_turbo.c
  cpuid.c
  profile.c
  memhdlr.c
  signalCombined.c)

#
# Add dynamic library
#
add_library(msr SHARED ${LIBMSR_SOURCES})

#
# Add static library with same base name as the dynamic lib.
#
add_library(msr-static STATIC ${LIBMSR_SOURCES})
set_target_properties(msr-static PROPERTIES OUTPUT_NAME "msr")

#
# Add math library for rapl
#
set (CMAKE_SHARED_LINKER_FLAGS "-lm")

#
# Install target should install the two library targets above.
#
install(TARGETS msr msr-static DESTINATION
"${CMAKE_INSTALL_PREFIX}/share/cmake/libmsr" EXPORT libmsr-libs)

# Headers are in ../include
include_directories(${PROJECT_SOURCE_DIR}/include)
