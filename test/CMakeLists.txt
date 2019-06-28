cmake_minimum_required(VERSION 2.8)

set(LIBMSR_TESTS
    power_unit_test
    libmsr_test
    pstate_test
    read_rapl_data
    translate_pkg_rapl_limit
    turbo_test
    libmsr_dump_data
    power_cap_test
)

foreach(TEST ${LIBMSR_TESTS})
    add_executable(${TEST} ${TEST}.c)
    set_target_properties(${TEST} PROPERTIES COMPILE_FLAGS "-g -Wall -D_GNU_SOURCE")
    target_link_libraries(${TEST} msr)
endforeach()

include_directories(${PROJECT_SOURCE_DIR}/include)
