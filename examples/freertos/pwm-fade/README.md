# freertos / pwm-fade

TIM3 CH1 PWM on PA6 faded by a task.

**Status: builds; untested on hardware.**

TIM3 runs at 1 MHz with ARR 999 (1 kHz PWM). The `fade` task changes the compare value by 1 % every 10 ms (`vTaskDelayUntil`),
a triangle from 0 to 100 % and back in 2 s. The `log` task prints the duty once a second. Wire an LED + resistor (330 R - 1 k) from PA6 to GND;
the on-board LEDs are not on timer channels.

Expected output:
```
[   1000] PA6 duty 100.0 %
```

## Build
```
../../../scripts/app example pwm-fade blackpill-f411   # from here (32_04_freertos/examples/freertos/pwm-fade); or by hand (from 32_04_freertos/):
. toolchain/freertos/env.sh
cmake -S examples/freertos/pwm-fade -B $FREERTOS_OUT/examples/pwm-fade/blackpill-f411 -G Ninja -DBOARD=blackpill-f411
cmake --build $FREERTOS_OUT/examples/pwm-fade/blackpill-f411 -j16        # -> pwm-fade.{elf,bin,hex}
```
Through the category's `mcu` tool: `MCU_JOBS=4 bash templates/tools/check.sh --only pwm-fade freertos`.
Flash: `st-flash --reset write pwm-fade.bin 0x08000000` (or openocd/pyocd/probe-rs, see `toolchain/stm32/README.md`). Console: `python -m serial.tools.miniterm /dev/ttyUSB0 115200` with a USB-UART on PA9/PA10.

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

PWM pin PA6 = TIM3_CH1 on all four: AF2 on F411 / G431 / H743, default mapping (no remap) on F103. Source: Zephyr hal_stm32 pinctrl data (`modules/hal/stm32/dts/st/<family>/stm32<part>-pinctrl.dtsi`, generated from ST's pin database)
(`tim3_ch1_pa6`, F103 `tim3_ch1_pwm_out_pa6`). On the mini H743, PA6 is also DCMI_PIXCLK of the camera connector (Zephyr
`mini_stm32h743.dts`): leave the camera off. That PA6 is broken out on the G431/H743 headers is unverified (no schematic re-read).
