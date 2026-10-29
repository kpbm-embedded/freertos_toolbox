# Host unit tests: CMake profile "Host-Tests" (buildtools gcc/g++, vendored GoogleTest, CTest).
# Add a test: put the code under lib/, add a library below and a mcu_add_host_test() line.
if(NOT MCU_ROOT)
  get_filename_component(MCU_ROOT "${CMAKE_CURRENT_LIST_DIR}/../../../toolchain" REALPATH)   # this ID's toolchain/clion (vendored GoogleTest)
endif()
include("${MCU_ROOT}/clion/cmake/McuHostTests.cmake")

add_library(ringbuf STATIC "${PROJECT_SOURCE_DIR}/lib/ringbuf/ringbuf.c")
target_include_directories(ringbuf PUBLIC "${PROJECT_SOURCE_DIR}/lib/ringbuf")

mcu_add_host_test(test_ringbuf test_ringbuf.cpp LIBS ringbuf)
