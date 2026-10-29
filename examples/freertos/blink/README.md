# freertos / blink

LED task with `vTaskDelayUntil` (drift-free 500 ms period) and a low-priority report task.

**Status: builds; untested on hardware.**

The `led` task toggles the board LED every 500 ms. The `report` task prints the toggle count and the LED task's stack
high-water mark (`uxTaskGetStackHighWaterMark`) every 5 s.

Expected output:
```
freertos blink on blackpill-f411
[   5000] blink: 10 toggles, led stack free 98 words
```

## Build
```
../../../scripts/app example blink blackpill-f411   # from here (32_04_freertos/examples/freertos/blink); or by hand (from 32_04_freertos/):
. toolchain/freertos/env.sh
cmake -S examples/freertos/blink -B $FREERTOS_OUT/examples/blink/blackpill-f411 -G Ninja -DBOARD=blackpill-f411
cmake --build $FREERTOS_OUT/examples/blink/blackpill-f411 -j16        # -> blink.{elf,bin,hex}
```
Through the category's `mcu` tool: `MCU_JOBS=4 bash templates/tools/check.sh --only blink freertos`.
Flash: `st-flash --reset write blink.bin 0x08000000` (or openocd/pyocd/probe-rs, see `toolchain/stm32/README.md`). Console: `python -m serial.tools.miniterm /dev/ttyUSB0 115200` with a USB-UART on PA9/PA10.

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
