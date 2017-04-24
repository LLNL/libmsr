project(libmsr C)
cmake_minimum_required(VERSION 2.8)

set(CMAKE_C_COMPILER "gcc")
set(CMAKE_CXX_COMPILER "g++")

# Add -Wall if it is not there already.
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -Wall -O3")

# Force default build type to be RelWithDebInfo
if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "RelWithDebInfo" CACHE STRING "Choose the type of build, options are: Debug Release RelWithDebInfo." FORCE)
endif()

# Build and run autoconf utility
set(LIBMSR_AUTOCONF ${CMAKE_BINARY_DIR}/._autoconf_)
execute_process(COMMAND gcc -o ${LIBMSR_AUTOCONF} ${CMAKE_SOURCE_DIR}/autoconf.c)
set(LIBMSR_TARGET_ARCH "" CACHE STRING "Force configuring for a particular architecture")
if (LIBMSR_TARGET_ARCH)
    execute_process(COMMAND ${LIBMSR_AUTOCONF} -f ${LIBMSR_TARGET_ARCH} WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
else()
    execute_process(COMMAND ${LIBMSR_AUTOCONF} WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
endif()

# Headers are in top level include directory
include_directories(${PROJECT_SOURCE_DIR}/include)

# Make libmsr findable
configure_file(
    ${PROJECT_SOURCE_DIR}/libmsr-config.cmake.in
    ${PROJECT_BINARY_DIR}/libmsr-config.cmake
    @ONLY
)

install(FILES ${PROJECT_BINARY_DIR}/libmsr-config.cmake DESTINATION
    "${CMAKE_INSTALL_PREFIX}/share/cmake/libmsr"
)
install(EXPORT libmsr-libs DESTINATION
    "${CMAKE_INSTALL_PREFIX}/share/cmake/libmsr"
)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)

add_subdirectory(src)
add_subdirectory(include)
add_subdirectory(dox)

add_subdirectory(test)
add_subdirectory(msrmod)
add_subdirectory(demoapps/powmon)

add_custom_target(distclean
    rm -f ._autoconf_
    rm -f CMakeCache.txt
    rm -f Makefile
    rm -f cmake_install.cmake
    rm -f demoapps/powmon/cmake_install.cmake
    rm -f demoapps/powmon/Makefile
    rm -f demoapps/powmon/power_wrapper_dynamic
    rm -f demoapps/powmon/power_wrapper_static
    rm -f demoapps/powmon/powmon
    rm -f dox/CMakeCache.txt
    rm -f dox/Makefile
    rm -f dox/cmake_install.cmake
    rm -f include/Makefile
    rm -f include/cmake_install.cmake
    rm -f include/master.h
    rm -f install_manifest.txt
    rm -f libmsr-config.cmake
    rm -f msrmod/cmake_install.cmake
    rm -f msrmod/Makefile
    rm -f msrmod/msrmod
    rm -f src/cmake_install.cmake
    rm -f src/Makefile
    rm -f test/CMakeCache.txt
    rm -f test/cmake_install.cmake
    rm -f test/Makefile
    rm -f test/dump-data
    rm -f test/libmsr-test
    rm -f test/pstate-test
    rm -f test/rapl-data
    rm -f test/translate
    rm -f test/turbo-test
    rm -f test/unit-test
    rm -rf CMakeFiles/
    rm -rf demoapps/powmon/CMakeFiles
    rm -rf dox/CMakeFiles/
    rm -rf dox/html/
    rm -rf dox/latex/
    rm -rf include/CMakeFiles/
    rm -rf lib/
    rm -rf msrmod/CMakeFiles
    rm -rf src/CMakeFiles/
    rm -rf test/CMakeFiles/
    rm -rf wrap/CMakeFiles/
    COMMENT "Cleaning up unwanted files" VERBATIM
)
