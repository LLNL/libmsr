project(libmsr C)
cmake_minimum_required(VERSION 2.8)

# Add -Wall if it is not there already.
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")

# Force default build type to be RelWithDebInfo
if (NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "RelWithDebInfo" CACHE STRING
    "Choose the type of build, options are: Debug Release RelWithDebInfo." FORCE)
endif()

#
# Add install RPATH to all executables & libraries.
#
set(CMAKE_SKIP_BUILD_RPATH FALSE)
set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
list(FIND CMAKE_PLATFORM_IMPLICIT_LINK_DIRECTORIES "${CMAKE_INSTALL_PREFIX}/lib" index)
if (index LESS 0)
  set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
endif()

# libmsr headers are in top level include directory
include_directories(${PROJECT_SOURCE_DIR}/include)

#Make libmsr findable
configure_file(
    ${PROJECT_SOURCE_DIR}/libmsr-config.cmake.in
    ${PROJECT_BINARY_DIR}/libmsr-config.cmake
    @ONLY)

message("Source Dir: ${PROJECT_SOURCE_DIR}")
message("Binary Dir: ${PROJECT_BINARY_DIR}")


install(FILES ${PROJECT_BINARY_DIR}/libmsr-config.cmake DESTINATION
"${CMAKE_INSTALL_PREFIX}/share/cmake/libmsr")
install(EXPORT libmsr-libs DESTINATION
"${CMAKE_INSTALL_PREFIX}/share/cmake/libmsr")

add_subdirectory(src)
add_subdirectory(cmd)

