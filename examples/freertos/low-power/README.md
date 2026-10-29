# freertos / low-power

Tickless idle: the kernel stops the tick and sleeps in WFI while every task is blocked.

**Status: builds; untested on hardware.**

`src/FreeRTOSConfig.h` shadows the shared config (it comes first on the include path and pulls in the shared file with `#include_next`)
and sets `configUSE_TICKLESS_IDLE 1`, `configEXPECTED_IDLE_TIME_BEFORE_SLEEP 5`. The port's default `vPortSuppressTicksAndSleep`
then reprograms SysTick for the whole expected idle time and executes WFI, and corrects the tick count on wake-up. The work is a
50 ms LED flash every 2 s plus a report every 10 s, so the core sleeps most of the time. `configPRE_SLEEP_PROCESSING` counts
the sleeps and the idle ticks each was planned for. This is Sleep mode: SysTick needs the core clock, so the clocks and the console keep
running. Deeper modes (Stop with an LPTIM/RTC wake-up) need a board-specific `vPortSuppressTicksAndSleep` and are not shown.

Expected output:
```
[  10000] last 10 s: 10 tickless sleeps planned for 9890 of 10000 ticks (~98 %; an interrupt can end one early)
```

## Build
```
../../../scripts/app example low-power blackpill-f411   # from here (32_04_freertos/examples/freertos/low-power); or by hand (from 32_04_freertos/):
. toolchain/freertos/env.sh
cmake -S examples/freertos/low-power -B $FREERTOS_OUT/examples/low-power/blackpill-f411 -G Ninja -DBOARD=blackpill-f411
cmake --build $FREERTOS_OUT/examples/low-power/blackpill-f411 -j16        # -> low-power.{elf,bin,hex}
```
Through the category's `mcu` tool: `MCU_JOBS=4 bash templates/tools/check.sh --only low-power freertos`.
Flash: `st-flash --reset write low-power.bin 0x08000000` (or openocd/pyocd/probe-rs, see `toolchain/stm32/README.md`). Console: `python -m serial.tools.miniterm /dev/ttyUSB0 115200` with a USB-UART on PA9/PA10.

Targets: blackpill-f411, weact-g431, mini-h743, bluepill-f103 (select with `-DBOARD=`).

Kernel: FreeRTOS-Kernel V11.3.2, heap_4, port per board (ARM_CM4F F411/G431, ARM_CM7/r0p1 H743, ARM_CM3 F103), heap 32/16/64/8 KiB
(`B_FREERTOS_HEAP` in each `board.cmake`); shared config `toolchain/freertos/common/FreeRTOSConfig.h` (1 kHz tick, ISRs that call
FromISR APIs at NVIC priority >= 5, here 6). Clocks: F411 100 MHz (HSE 25), G431 144 MHz (HSE 8), H743 400 MHz (HSE 25), F103 72 MHz (HSE 8).

## Pins
| Board | LED | Console (USART1) | Source |
|---|---|---|---|
| blackpill-f411 | PC13, active low | PA9 TX / PA10 RX | `toolchain/stm32/boards/blackpill-f411/board.cmake` (WeAct schematic; Zephyr `blackpill_f411ce.dts`) |
| weact-g431 | PC6, active high | PA9 TX / PA10 RX | `boards/weact-g431/board.cmake` (Zephyr `weact_stm32g431_core.dts` led0; that DTS uses USART2 PA2/PA3 as its console, this board layer uses USART1) |
| mini-h743 | PE3, active high | PA9 TX / PA10 RX | `boards/mini-h743/board.cmake` (Zephyr `mini_stm32h743.dts`) |
| bluepill-f103 | PC13, active low | PA9 TX / PA10 RX | `boards/bluepill-f103/board.cmake` (Zephyr `stm32_min_dev` blue overlay) |
