cmake_minimum_required(VERSION 2.8)

include_directories(${PROJECT_SOURCE_DIR}/include)

add_executable (msrmod msrmod.c)
set_target_properties(${execname} PROPERTIES COMPILE_FLAGS "-g -Wall -D_GNU_SOURCE")
target_link_libraries (msrmod msr)

install(TARGETS msrmod EXPORT libmsr-libs DESTINATION
    "${CMAKE_INSTALL_PREFIX}/bin"
)
