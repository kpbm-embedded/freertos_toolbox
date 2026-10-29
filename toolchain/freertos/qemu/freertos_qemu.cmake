# FreeRTOS-Kernel on qemu mps2-an385 (Cortex-M3, ARM_CM3 port). Use from an example or template:
#   set(FREERTOS_QEMU $ENV{FREERTOS_KERNEL}/../qemu)
#   set(CMAKE_TOOLCHAIN_FILE ${FREERTOS_QEMU}/arm-none-eabi.cmake)    # before project()
#   project(app C ASM)
#   include(${FREERTOS_QEMU}/freertos_qemu.cmake)
#   add_executable(app src/main.c)
#   freertos_qemu(app [HEAP heap_4|heap_1|...|none] [CLI] [SOURCES ...] [INCLUDES ...] [DEFINES ...])
# app_config.h in the project's src/ (or INCLUDES) overrides FreeRTOSConfig.h values. Produces app.elf/.bin/.hex,
# app.map and prints the size. Run: scripts/app qemu <app.elf> (or scripts/app run <example>).
set(FREERTOS_QEMU ${CMAKE_CURRENT_LIST_DIR})
if(NOT DEFINED ENV{FREERTOS_KERNEL} OR NOT EXISTS $ENV{FREERTOS_KERNEL}/tasks.c)
  message(FATAL_ERROR "FREERTOS_KERNEL not set or not installed: source toolchain/freertos/env.sh, then scripts/app setup")
endif()
set(FREERTOS_KERNEL_DIR $ENV{FREERTOS_KERNEL})
if(BOARD AND NOT BOARD STREQUAL "mps2-an385")
  message(FATAL_ERROR "this project runs on qemu: BOARD must be mps2-an385 (got ${BOARD})")
endif()
set(QEMU_MCU_FLAGS -mcpu=cortex-m3 -mthumb)

function(freertos_qemu tgt)
  cmake_parse_arguments(A "CLI" "HEAP" "SOURCES;INCLUDES;DEFINES" ${ARGN})
  if(NOT A_HEAP)
    set(A_HEAP heap_4)
  endif()
  set(K ${FREERTOS_KERNEL_DIR})
  target_sources(${tgt} PRIVATE ${K}/tasks.c ${K}/list.c ${K}/queue.c ${K}/timers.c ${K}/event_groups.c
    ${K}/stream_buffer.c ${K}/portable/GCC/ARM_CM3/port.c
    ${FREERTOS_QEMU}/startup.c ${FREERTOS_QEMU}/board.c ${FREERTOS_QEMU}/rtos_hooks.c ${A_SOURCES})
  if(NOT A_HEAP STREQUAL "none")
    target_sources(${tgt} PRIVATE ${K}/portable/MemMang/${A_HEAP}.c)
  endif()
  if(A_CLI)
    set(CLI_DIR ${K}/../plus-cli)
    if(NOT EXISTS ${CLI_DIR}/FreeRTOS_CLI.c)
      message(FATAL_ERROR "FreeRTOS+CLI not installed: scripts/app setup")
    endif()
    target_sources(${tgt} PRIVATE ${CLI_DIR}/FreeRTOS_CLI.c)
    target_include_directories(${tgt} PRIVATE ${CLI_DIR})
  endif()
  target_include_directories(${tgt} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src ${A_INCLUDES} ${FREERTOS_QEMU}
    ${K}/include ${K}/portable/GCC/ARM_CM3)
  target_compile_definitions(${tgt} PRIVATE ${A_DEFINES})
  set_target_properties(${tgt} PROPERTIES SUFFIX .elf)
  target_compile_options(${tgt} PRIVATE ${QEMU_MCU_FLAGS} -Og -g3 -ffunction-sections -fdata-sections -Wall -Wextra
    -Wno-unused-parameter)
  target_link_options(${tgt} PRIVATE ${QEMU_MCU_FLAGS} -T${FREERTOS_QEMU}/mps2_an385.ld --specs=nano.specs
    -Wl,--gc-sections -Wl,-Map=${CMAKE_CURRENT_BINARY_DIR}/${tgt}.map -Wl,--no-warn-rwx-segments)
  add_custom_command(TARGET ${tgt} POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:${tgt}> ${tgt}.bin
    COMMAND ${CMAKE_OBJCOPY} -O ihex $<TARGET_FILE:${tgt}> ${tgt}.hex
    COMMAND ${CMAKE_SIZE} $<TARGET_FILE:${tgt}>
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
endfunction()
