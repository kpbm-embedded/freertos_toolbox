# FreeRTOS-Kernel (V11.x, heap_4) on top of the mcu/stm32 board layer. Use after include(board.cmake):
#   include($ENV{FREERTOS_KERNEL}/../common/freertos.cmake)
#   add_executable(app src/main.c)
#   freertos_board(app [MODULES tim ...] [DEFINES ...])
# Source freertos/env.sh first (it also sources stm32/env.sh). Console: rtos_print()/rtos_log() from rtos_glue.h.
set(FREERTOS_COMMON ${CMAKE_CURRENT_LIST_DIR})
function(freertos_board tgt)
  cmake_parse_arguments(A "" "" "MODULES;DEFINES" ${ARGN})
  set(K $ENV{FREERTOS_KERNEL})
  if(NOT K OR NOT EXISTS ${K}/tasks.c)
    message(FATAL_ERROR "FREERTOS_KERNEL not set: source mcu/freertos/env.sh")
  endif()
  stm32_board(${tgt} NO_SYSTICK MODULES ${A_MODULES} DEFINES FREERTOS_HEAP_BYTES=${B_FREERTOS_HEAP} ${A_DEFINES})
  target_sources(${tgt} PRIVATE ${K}/tasks.c ${K}/list.c ${K}/queue.c ${K}/timers.c ${K}/event_groups.c ${K}/stream_buffer.c
    ${K}/portable/GCC/${B_FREERTOS_PORT}/port.c ${K}/portable/MemMang/heap_4.c ${FREERTOS_COMMON}/rtos_glue.c)
  target_include_directories(${tgt} PRIVATE ${FREERTOS_COMMON} ${K}/include ${K}/portable/GCC/${B_FREERTOS_PORT})
endfunction()
