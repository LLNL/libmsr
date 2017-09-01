cmake_minimum_required(VERSION 2.8)

include_directories(${PROJECT_SOURCE_DIR}/include)

add_executable (power-unit-test power_unit_test.c)
set_target_properties(${execname} PROPERTIES COMPILE_FLAGS "-g -Wall -D_GNU_SOURCE")
target_link_libraries (power-unit-test msr)

add_executable (libmsr-test libmsr_test.c)
set_target_properties(${execname} PROPERTIES COMPILE_FLAGS "-g -Wall -D_GNU_SOURCE")
target_link_libraries (libmsr-test msr)

add_executable (pstate-test pstate_test.c)
set_target_properties(${execname} PROPERTIES COMPILE_FLAGS "-g -Wall -D_GNU_SOURCE")
target_link_libraries (pstate-test msr)

add_executable (rapl-data read_rapl_data.c)
set_target_properties(${execname} PROPERTIES COMPILE_FLAGS "-g -Wall -D_GNU_SOURCE")
target_link_libraries (rapl-data msr)

add_executable (translate translate_pkg_rapl_limit.c)
set_target_properties(${execname} PROPERTIES COMPILE_FLAGS "-g -Wall -D_GNU_SOURCE")
target_link_libraries (translate msr)

add_executable (turbo-test turbo_test.c)
set_target_properties(${execname} PROPERTIES COMPILE_FLAGS "-g -Wall -D_GNU_SOURCE")
target_link_libraries (turbo-test msr)

add_executable (dump-data libmsr_dump_data.c)
set_target_properties(${execname} PROPERTIES COMPILE_FLAGS "-g -Wall -D_GNU_SOURCE")
target_link_libraries (dump-data msr)

add_executable (power-cap-test power_cap_test.c)
set_target_properties(${execname} PROPERTIES COMPILE_FLAGS "-g -Wall -D_GNU_SOURCE")
target_link_libraries (power-cap-test msr)
