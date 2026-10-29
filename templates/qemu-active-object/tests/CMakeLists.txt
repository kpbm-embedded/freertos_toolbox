cmake_minimum_required(VERSION 3.20)
# Host unit tests for lib/ (GoogleTest from the freertos-box toolchain, built with the host gcc/g++).
# Run: scripts/app test <app dir>
project(@NAME@_host_tests C CXX)
enable_testing()
if(NOT DEFINED ENV{FREERTOS_KERNEL})
  message(FATAL_ERROR "source <freertos-box>/toolchain/freertos/env.sh first (scripts/app does it)")
endif()
include($ENV{FREERTOS_KERNEL}/../../clion/cmake/McuHostTests.cmake)
add_library(fsm STATIC ${CMAKE_CURRENT_SOURCE_DIR}/../lib/fsm.c)
target_include_directories(fsm PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../lib)
mcu_add_host_test(test_fsm test_fsm.cpp LIBS fsm)
