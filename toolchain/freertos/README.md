# 32_04_freertos/toolchain/freertos
FreeRTOS-Kernel V11.3.2 (github FreeRTOS/FreeRTOS-Kernel, shallow). Port GCC/ARM_CM4F, heap_4. Reuses `../stm32/STM32CubeF4` v1.28.3 and `../stm32/examples/common`.
Example `blinky-f411-freertos`: task "led" toggles PC13 every 500 ms, task "uart" prints via printf on USART1 TX (PA9, 115200 8N1) every second. SysTick is owned by FreeRTOS; HAL_GetTick uses DWT before scheduler start and xTaskGetTickCount after.
Build: `mcu build freertos blackpill-f411` (the category's `mcu` tool reaches this folder via its `freertos` symlink; -> `$MCU_OUT_BASE/freertos/blackpill-f411/blinky-f411-freertos/`). Apps/examples of this ID: `../../scripts/app` (see ../../README.md). Size: text 11700, data 112, bss 35496 (32 KiB heap).
Flash/monitor: same probes as ../stm32/README.md (`mcu flash freertos blackpill-f411 blinky-f411-freertos st-flash`); console: `mcu monitor freertos /dev/ttyUSB0 115200` with a USB-UART on PA9.
Related: FreeRTOS is also used by the K210 (kendryte-freertos-sdk) and ESP-IDF (its own FreeRTOS fork); both are SDKs of the `mcu` tool, not part of this ID.

## Self-contained
env.sh resolves its own physical path (so it stays in this ID even when reached through a symlink) and sources `../host/env.sh`, `../stm32/env.sh` (Arm toolchain, Cube F4) and adds FreeRTOS-Kernel. Nothing comes from OS packages beyond glibc and sysbin utilities.

## Templates and examples
`common/` has the shared `FreeRTOSConfig.h`, `rtos_glue.[ch]` (HAL timebase on DWT/RTOS tick, mutex-protected `rtos_log`) and `freertos.cmake` (`freertos_board(<tgt>)`), used with `../stm32/boards/<board>` by `../../examples/freertos/*` (build one with `../../scripts/app example <name>`). Ports: ARM_CM4F (F411, G431), ARM_CM7/r0p1 (H743), ARM_CM3 (F103).
