project(libmsr C)
cmake_minimum_required(VERSION 2.8)

# Add -Wall if it is not there already.
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -O3")

# Force default build type to be RelWithDebInfo
if (NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "RelWithDebInfo" CACHE STRING
    "Choose the type of build, options are: Debug Release RelWithDebInfo." FORCE)
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

find_package(Doxygen)
if (DOXYGEN_FOUND)
    add_subdirectory(dox)
endif(DOXYGEN_FOUND)

add_subdirectory(test)

add_custom_target(distclean
    rm -f CMakeCache.txt
    rm -f Makefile
    rm -f _autoconf_
    rm -f cmake_install.cmake
    rm -f demoapps/powmon/Makefile.in
    rm -f demoapps/powmon/aclocal.m4
    rm -f demoapps/powmon/config.h
    rm -f demoapps/powmon/config.h.in
    rm -f demoapps/powmon/config.log
    rm -f demoapps/powmon/config.status
    rm -f demoapps/powmon/configure
    rm -f demoapps/powmon/depcomp
    rm -f demoapps/powmon/install-sh
    rm -f demoapps/powmon/missing
    rm -f demoapps/powmon/src/Makefile.in
    rm -f demoapps/powmon/src/Makefile
    rm -f demoapps/powmon/src/power_wrapper_dynamic
    rm -f demoapps/powmon/src/power_wrapper_static
    rm -f demoapps/powmon/src/powmon
    rm -f demoapps/powmon/stamp-h1
    rm -f dox/CMakeCache.txt
    rm -f dox/Makefile
    rm -f dox/cmake_install.cmake
    rm -f include/Makefile
    rm -f include/cmake_install.cmake
    rm -f include/master.h
    rm -f install_manifest.txt
    rm -f libmsr-config.cmake
    rm -f msrmod/msrmod
    rm -f platform_headers/master.h
    rm -f src/Makefile
    rm -f src/cmake_install.cmake
    rm -f test/CMakeCache.txt
    rm -f test/Makefile
    rm -f test/cmake_install.cmake
    rm -f test/dump-data
    rm -f test/libmsr-test
    rm -f test/pstate-test
    rm -f test/rapl-data
    rm -f test/translate
    rm -f test/turbo-test
    rm -f test/unit-test
    rm -rf BUILD/
    rm -rf CMakeFiles/
    rm -rf demoapps/powmon/autom4te.cache/
    rm -rf demoapps/powmon/src/.deps/
    rm -rf dox/CMakeFiles/
    rm -rf dox/html/
    rm -rf dox/latex/
    rm -rf include/CMakeFiles/
    rm -rf lib/
    rm -rf src/CMakeFiles/
    rm -rf test/CMakeFiles/
    rm -rf wrap/CMakeFiles/
    COMMENT "Cleaning up unwanted files" VERBATIM
)
