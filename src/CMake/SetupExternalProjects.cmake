# Necessary for ExternalProject_Add
include(ExternalProject)

message(STATUS " [+] Adding external project: hwloc")
ExternalProject_Add(libhwloc
  GIT_REPOSITORY https://github.com/open-mpi/hwloc.git
  UPDATE_COMMAND ""
  PATCH_COMMAND ""
  SOURCE_DIR ${CMAKE_CURRENT_BINARY_DIR}/thirdparty_builtin/hwloc
  CONFIGURE_COMMAND ${CMAKE_CURRENT_BINARY_DIR}/thirdparty_builtin/hwloc/autogen.sh && ${CMAKE_CURRENT_BINARY_DIR}/thirdparty_builtin/hwloc/configure --prefix=${CMAKE_CURRENT_BINARY_DIR}/thirdparty_builtin/libhwloc-prefix
  PREFIX ${CMAKE_CURRENT_BINARY_DIR}/thirdparty_builtin/libhwloc-prefix
  BUILD_COMMAND make
  INSTALL_COMMAND make install
  BUILD_IN_SOURCE 1
)

ExternalProject_Get_property(libhwloc INSTALL_DIR)
set(HWLOC_INCLUDE_DIRS
  ${INSTALL_DIR}/include
  CACHE INTERNAL "")

set(HWLOC_LIBRARY
  ${INSTALL_DIR}/lib/${CMAKE_SHARED_LIBRARY_PREFIX}hwloc${CMAKE_SHARED_LIBRARY_SUFFIX}
  CACHE INTERNAL "")

include_directories(${HWLOC_INCLUDES})
