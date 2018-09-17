###############################################################################
# Setup Hwloc
# This file defines:
#  HWLOC_FOUND - If Hwloc was found
#  HWLOC_INCLUDE_DIRS - The Hwloc include directories
#  
#  If found, the Hwloc CMake targets will also be imported
###############################################################################

# first check for HWLOC_DIR

if(NOT HWLOC_DIR)
    MESSAGE(FATAL_ERROR "Hwloc support needs explicit HWLOC_DIR")
endif()

MESSAGE(STATUS "Looking for Hwloc using HWLOC_DIR = ${HWLOC_DIR}")

include(${HWLOC_DIR}/include)

set(HWLOC_FOUND TRUE)
set(HWLOC_LIBRARY ${HWLOC_DIR}/lib/libhwloc.so)
set(HWLOC_INCLUDES ${HWLOC_DIR}/include)

message(STATUS "FOUND Hwloc at ${HWLOC_DIR}")
message(STATUS "HWLOC_INCLUDES = ${HWLOC_INCLUDES}")
