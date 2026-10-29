# freertos / stack-overflow-hook

configCHECK_FOR_STACK_OVERFLOW=2 demo: the hook reports the offending task and resets.

**Status: builds; untested on hardware.**

Task `victim` recurses with a delay at every level until it overruns its stack; the kernel's pattern check at context switch calls `vApplicationStackOverflowHook`, which prints the task name, flashes the LED and resets the MCU. Detection is best-effort (checks happen at context switches), so memory next to the stack may already be damaged: reset, do not continue.

Expected output: `victim: depth N ...` lines, then `*** STACK OVERFLOW in task 'victim' - resetting ***`, repeating after the reset.

## Build
```
../../../scripts/app example stack-overflow-hook blackpill-f411   # from here; or by hand (from 32_04_freertos/):
. toolchain/freertos/env.sh
cmake -S examples/freertos/stack-overflow-hook -B $FREERTOS_OUT/examples/stack-overflow-hook/blackpill-f411 -G Ninja -DBOARD=blackpill-f411
cmake --build $FREERTOS_OUT/examples/stack-overflow-hook/blackpill-f411 -j16        # -> stack-overflow-hook.{elf,bin,hex}
```
Flash: `st-flash --reset write stack-overflow-hook.bin 0x08000000` (or openocd/pyocd/probe-rs, see `toolchain/stm32/README.md`). Console: `python -m serial.tools.miniterm /dev/ttyUSB0 115200` with a USB-UART on PA9/PA10.

Targets: blackpill-f411, weact-g431, mini-h743, bluepill-f103 (select with `-DBOARD=`).

Pin and clock sources: the `board.cmake` of each board in `toolchain/stm32/boards/<board>/` cites them (cross-checked with the Zephyr board DTS in this
workspace; vendor schematics not re-read). Kernel: FreeRTOS-Kernel V11.3.2, port GCC/ARM_CM4F (F411, G431), ARM_CM7/r0p1 (H743), ARM_CM3 (F103), heap_4 (heap 32/16/64/8 KiB); shared config in `toolchain/freertos/common/FreeRTOSConfig.h`
(1 kHz tick, 4 priority bits, syscall ceiling 5). HAL timebase: DWT before the scheduler, RTOS tick after (`common/rtos_glue.c`).
Pins: BlackPill F411 LED PC13 (active low), console PA9 TX / PA10 RX; WeAct G431 LED PC6 (active high), mini H743 LED PE3 (active high), BluePill F103 LED PC13 (active low); console USART1 PA9 / PA10 on all, 115200 8N1.
