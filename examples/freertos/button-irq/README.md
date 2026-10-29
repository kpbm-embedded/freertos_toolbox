# freertos / button-irq

Button EXTI interrupt -> task notification -> debounced LED toggle.

**Status: builds; untested on hardware.**

`board_button_init(1)` arms the EXTI line on the press edge at NVIC priority 6. The ISR hook (`board_button_hook`) only calls
`vTaskNotifyGiveFromISR`; the `button` task wakes, re-checks the pin after 30 ms (debounce), toggles the LED, logs, waits for the release
and discards bounce notifications.

Expected output:
```
[   1234] press #1 (3 edge interrupts so far)
```

## Build
```
../../../scripts/app example button-irq blackpill-f411   # from here (32_04_freertos/examples/freertos/button-irq); or by hand (from 32_04_freertos/):
. toolchain/freertos/env.sh
cmake -S examples/freertos/button-irq -B $FREERTOS_OUT/examples/button-irq/blackpill-f411 -G Ninja -DBOARD=blackpill-f411
cmake --build $FREERTOS_OUT/examples/button-irq/blackpill-f411 -j16        # -> button-irq.{elf,bin,hex}
```
Through the category's `mcu` tool: `MCU_JOBS=4 bash templates/tools/check.sh --only button-irq freertos`.
Flash: `st-flash --reset write button-irq.bin 0x08000000` (or openocd/pyocd/probe-rs, see `toolchain/stm32/README.md`). Console: `python -m serial.tools.miniterm /dev/ttyUSB0 115200` with a USB-UART on PA9/PA10.

Targets: blackpill-f411, weact-g431, mini-h743 (select with `-DBOARD=`).

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

| Board | Button | Source |
|---|---|---|
| blackpill-f411 | KEY PA0, active low, pull-up, EXTI0 | `boards/blackpill-f411/board.cmake` (WeAct schematic; Zephyr `blackpill_f411ce.dts`) |
| weact-g431 | PC13, active high, pull-down, EXTI15_10 | `boards/weact-g431/board.cmake` (Zephyr `weact_stm32g431_core.dts` button0) |
| mini-h743 | PC13 (K1), active high, pull-down, EXTI15_10 | `boards/mini-h743/board.cmake` (Zephyr `mini_stm32h743.dts` user_button) |

bluepill-f103: N/A, the BluePill has no user button (board layer `BOARD_HAS_BUTTON 0`; a BluePill+ key is unverified).
