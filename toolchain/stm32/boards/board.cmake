# Shared STM32 board layer (HAL + CMSIS + startup + board.c + generated linker script/hal_conf).
# Use from a project (toolchain file must be set BEFORE project()):
#   set(STM32_BOARDS "$ENV{STM32_CUBE_F4}/../boards")      # needs: . mcu/stm32/env.sh
#   set(CMAKE_TOOLCHAIN_FILE ${STM32_BOARDS}/arm-none-eabi.cmake)
#   set(BOARD blackpill-f411 CACHE STRING "board")
#   project(app C ASM)  include(${STM32_BOARDS}/board.cmake)
#   add_executable(app main.c)  stm32_board(app MODULES adc i2c)
# Options: MODULES <hal modules beyond the always-on set>, DEFINES <extra defs for board+HAL>, NO_SYSTICK (RTOS owns SysTick),
#          EXTRA_LINK <libs>. Produces <tgt>.elf/.bin/.hex and prints size.
set(STM32_BOARDS_DIR ${CMAKE_CURRENT_LIST_DIR})
if(NOT BOARD)
  message(FATAL_ERROR "set BOARD (blackpill-f411 weact-g431 mini-h743 bluepill-f103)")
endif()
if(NOT EXISTS ${STM32_BOARDS_DIR}/${BOARD}/board.cmake)
  message(FATAL_ERROR "unknown BOARD '${BOARD}' (see ${STM32_BOARDS_DIR}/*/)")
endif()
include(${STM32_BOARDS_DIR}/${BOARD}/board.cmake)   # sets B_FAMILY B_DEVICE B_MCU_FLAGS B_HSE_HZ B_FLASH_* B_RAM_* ...
string(TOLOWER ${B_FAMILY} _fl)                      # f4
set(_CUBE $ENV{STM32_CUBE_${B_FAMILY}})
if(NOT _CUBE OR NOT EXISTS ${_CUBE}/Drivers)
  message(FATAL_ERROR "STM32_CUBE_${B_FAMILY} not set: source mcu/stm32/env.sh")
endif()
set(_HAL ${_CUBE}/Drivers/STM32${B_FAMILY}xx_HAL_Driver)
set(_DEV ${_CUBE}/Drivers/CMSIS/Device/ST/STM32${B_FAMILY}xx)

function(stm32_board tgt)
  cmake_parse_arguments(A "NO_SYSTICK" "" "MODULES;DEFINES;EXTRA_LINK" ${ARGN})
  set(gen ${CMAKE_BINARY_DIR}/${tgt}_gen)
  file(MAKE_DIRECTORY ${gen})
  # --- HAL config: template with every module disabled, then ours enabled
  file(READ ${_HAL}/Inc/stm32${_fl}xx_hal_conf_template.h conf)
  string(REGEX REPLACE "(^|\n)[ \t]*(/\\*[ \t]*)?#define[ \t]+(HAL_[A-Z0-9_]+_MODULE_ENABLED)[ \t]*(\\*/)?" "\\1/* #define \\3 */" conf "${conf}")
  set(mods cortex dma flash gpio pwr rcc ${B_EXTRA_MODULES} ${A_MODULES})
  list(REMOVE_DUPLICATES mods)
  set(pre "#define HAL_MODULE_ENABLED\n")
  foreach(m ${mods})
    string(TOUPPER ${m} M)
    string(APPEND pre "#define HAL_${M}_MODULE_ENABLED\n")
  endforeach()
  file(WRITE ${gen}/stm32${_fl}xx_hal_conf.h "${pre}${conf}")
  # --- sources
  set(src ${_HAL}/Src/stm32${_fl}xx_hal.c)
  foreach(m ${mods})
    foreach(sfx "" "_ex")
      set(f ${_HAL}/Src/stm32${_fl}xx_hal_${m}${sfx}.c)
      if(EXISTS ${f})
        list(APPEND src ${f})
      endif()
    endforeach()
    if(m STREQUAL "pcd")
      list(APPEND src ${_HAL}/Src/stm32${_fl}xx_ll_usb.c)
    endif()
  endforeach()
  list(APPEND src ${_DEV}/Source/Templates/system_stm32${_fl}xx.c
       ${_DEV}/Source/Templates/gcc/startup_${B_STARTUP}.s
       ${STM32_BOARDS_DIR}/board.c)
  set(B_HEAP 0x400 CACHE STRING "min heap")
  set(B_STACK 0x800 CACHE STRING "min stack")
  configure_file(${STM32_BOARDS_DIR}/link.ld.in ${gen}/link.ld @ONLY)
  set(lib ${tgt}_board)
  add_library(${lib} OBJECT ${src})
  target_compile_definitions(${lib} PUBLIC ${B_DEVICE} USE_HAL_DRIVER HSE_VALUE=${B_HSE_HZ}U BOARD_${B_FAMILY}=1 BOARD_ID_${B_ID}=1 ${A_DEFINES})
  if(A_NO_SYSTICK)
    target_compile_definitions(${lib} PUBLIC BOARD_NO_SYSTICK=1)
  endif()
  target_include_directories(${lib} PUBLIC ${gen} ${STM32_BOARDS_DIR} ${STM32_BOARDS_DIR}/${BOARD}
    ${_HAL}/Inc ${_DEV}/Include ${_CUBE}/Drivers/CMSIS/Core/Include ${_CUBE}/Drivers/CMSIS/Include)
  target_compile_options(${lib} PUBLIC ${B_MCU_FLAGS} -Os -g -ffunction-sections -fdata-sections -Wall)
  target_link_options(${lib} PUBLIC ${B_MCU_FLAGS} -T${gen}/link.ld --specs=nano.specs --specs=nosys.specs
    -Wl,--gc-sections -Wl,--no-warn-rwx-segments -Wl,-Map=${CMAKE_BINARY_DIR}/${tgt}.map -Wl,--print-memory-usage)
  target_link_libraries(${tgt} PUBLIC ${lib} ${A_EXTRA_LINK})
  set_target_properties(${tgt} PROPERTIES SUFFIX .elf)
  add_custom_command(TARGET ${tgt} POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:${tgt}> ${tgt}.bin
    COMMAND ${CMAKE_OBJCOPY} -O ihex $<TARGET_FILE:${tgt}> ${tgt}.hex
    COMMAND ${CMAKE_SIZE} $<TARGET_FILE:${tgt}>)
endfunction()
