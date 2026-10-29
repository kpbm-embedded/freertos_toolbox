# mcu/clion/cmake/McuHostTests.cmake - host unit tests for the "Host-Tests" CMake profile.
#
# GoogleTest 1.18.0 is vendored in ../third_party (official release tarball, sha256 in
# ../third_party/dl/*.sha256); nothing is fetched at configure time. Tests are compiled with the
# buildtools gcc/g++ (the profile passes -DCMAKE_C_COMPILER/-DCMAKE_CXX_COMPILER) and registered
# with CTest through gtest_discover_tests, so CLion's test runner shows each TEST().
#
#   mcu_add_host_test(<name> <sources...> [LIBS <libs...>])
include_guard(GLOBAL)

if(NOT MCU_ROOT)
  get_filename_component(MCU_ROOT "${CMAKE_CURRENT_LIST_DIR}/../.." ABSOLUTE)
endif()
set(MCU_GTEST_DIR "${MCU_ROOT}/clion/third_party/googletest-1.18.0" CACHE PATH "vendored GoogleTest")

if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 17)
endif()
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
set(BUILD_GMOCK OFF CACHE BOOL "" FORCE)
add_subdirectory("${MCU_GTEST_DIR}" "${CMAKE_BINARY_DIR}/_deps/googletest" EXCLUDE_FROM_ALL SYSTEM)
include(GoogleTest)

function(mcu_add_host_test name)
  cmake_parse_arguments(T "" "" "LIBS" ${ARGN})
  add_executable(${name} ${T_UNPARSED_ARGUMENTS})
  target_link_libraries(${name} PRIVATE ${T_LIBS} GTest::gtest_main)
  target_compile_options(${name} PRIVATE -Wall -Wextra)
  gtest_discover_tests(${name} DISCOVERY_MODE PRE_TEST)
endfunction()
