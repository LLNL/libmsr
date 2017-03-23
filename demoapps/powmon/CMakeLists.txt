cmake_minimum_required(VERSION 2.8)

set(CMAKE_C_FLAGS "-pthread")
set(CMAKE_CXX_FLAGS "-pthread")

set(POWMON_SOURCES
    highlander.c
    powmon.c
    rapl2.c)
set(POWER_WRAPPER_STATIC_SOURCES
    highlander.c
    power_wrapper_static.c
    rapl2.c)
set(POWER_WRAPPER_DYNAMIC_SOURCES
    highlander.c
    power_wrapper_dynamic.c
    rapl2.c)

include_directories(${PROJECT_SOURCE_DIR}/include)

add_executable (powmon ${POWMON_SOURCES})
add_executable (power_wrapper_static ${POWER_WRAPPER_STATIC_SOURCES})
add_executable (power_wrapper_dynamic ${POWER_WRAPPER_DYNAMIC_SOURCES})

target_link_libraries (powmon msr)
target_link_libraries (power_wrapper_static msr)
target_link_libraries (power_wrapper_dynamic msr)

install(TARGETS powmon EXPORT libmsr-libs DESTINATION
    "${CMAKE_INSTALL_PREFIX}/bin"
)
install(TARGETS power_wrapper_static EXPORT libmsr-libs DESTINATION
    "${CMAKE_INSTALL_PREFIX}/bin"
)
install(TARGETS power_wrapper_dynamic EXPORT libmsr-libs DESTINATION
    "${CMAKE_INSTALL_PREFIX}/bin"
)
