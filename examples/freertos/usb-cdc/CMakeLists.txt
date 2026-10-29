cmake_minimum_required(VERSION 3.20)
# Needs: . toolchain/freertos/env.sh   (Arm GNU toolchain, cmake, ninja, STM32 Cube paths, FREERTOS_KERNEL)
if(NOT DEFINED ENV{STM32_CUBE_F4} OR NOT DEFINED ENV{FREERTOS_KERNEL})
  message(FATAL_ERROR "source 32_04_freertos/toolchain/freertos/env.sh first")
endif()
set(STM32_BOARDS "$ENV{STM32_CUBE_F4}/../boards" CACHE PATH "toolchain/stm32/boards (shared board layer)")
set(CMAKE_TOOLCHAIN_FILE ${STM32_BOARDS}/arm-none-eabi.cmake)
set(BOARD "blackpill-f411" CACHE STRING "blackpill-f411 | weact-g431 | mini-h743 | bluepill-f103")
project(usb-cdc C ASM)
include(${STM32_BOARDS}/board.cmake)
include($ENV{FREERTOS_KERNEL}/../common/freertos.cmake)
add_executable(usb-cdc src/main.c src/usbd_conf.c src/usbd_desc.c)
# BOARD_USB48: the F411 clock tree switches to 96 MHz so PLLQ gives 48 MHz (other boards ignore it).
freertos_board(usb-cdc MODULES pcd DEFINES BOARD_USB48=1)
# ST USB Device library (Core + CDC class) from the board family's own Cube pack.
set(USBD $ENV{STM32_CUBE_${B_FAMILY}}/Middlewares/ST/STM32_USB_Device_Library)
target_sources(usb-cdc PRIVATE ${USBD}/Core/Src/usbd_core.c ${USBD}/Core/Src/usbd_ctlreq.c ${USBD}/Core/Src/usbd_ioreq.c
  ${USBD}/Class/CDC/Src/usbd_cdc.c)
target_include_directories(usb-cdc PRIVATE src ${USBD}/Core/Inc ${USBD}/Class/CDC/Inc)
