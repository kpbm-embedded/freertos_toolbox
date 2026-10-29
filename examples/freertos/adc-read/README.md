# freertos / adc-read

ADC1 on PA1 sampled by a task; the end-of-conversion interrupt notifies it.

**Status: builds; untested on hardware.**

Every 500 ms the `adc` task starts one conversion (`HAL_ADC_Start_IT`) and blocks on `ulTaskNotifyTake` (10 ms timeout). The ADC
interrupt (priority 6) stores the result and calls `vTaskNotifyGiveFromISR`. The task logs raw value and millivolts (VDDA assumed 3.3 V).
Resolution 12 bit (F411, G431, F103) or 16 bit (H743). Clocks: F411 PCLK2/4 = 25 MHz; G431 HCLK/4 = 36 MHz; H743 HCLK/4 = 50 MHz;
F103 PCLK2/6 = 12 MHz (max 14). G431, H743 and F103 run the ADC self-calibration first.

Expected output:
```
[    500] PA1 raw= 2048  1650 mV
```

## Build
```
../../../scripts/app example adc-read blackpill-f411   # from here (32_04_freertos/examples/freertos/adc-read); or by hand (from 32_04_freertos/):
. toolchain/freertos/env.sh
cmake -S examples/freertos/adc-read -B $FREERTOS_OUT/examples/adc-read/blackpill-f411 -G Ninja -DBOARD=blackpill-f411
cmake --build $FREERTOS_OUT/examples/adc-read/blackpill-f411 -j16        # -> adc-read.{elf,bin,hex}
```
Through the category's `mcu` tool: `MCU_JOBS=4 bash templates/tools/check.sh --only adc-read freertos`.
Flash: `st-flash --reset write adc-read.bin 0x08000000` (or openocd/pyocd/probe-rs, see `toolchain/stm32/README.md`). Console: `python -m serial.tools.miniterm /dev/ttyUSB0 115200` with a USB-UART on PA9/PA10.

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

Analog input PA1: F411 ADC1_IN1, F103 ADC1_IN1, G431 ADC1_IN2, H743 ADC1_INP17. Source: Zephyr hal_stm32 pinctrl data (`modules/hal/stm32/dts/st/<family>/stm32<part>-pinctrl.dtsi`, generated from ST's pin database) (`adc1_in1_pa1`, `adc1_in2_pa1`,
`adc1_inp17_pa1`); F411 also Zephyr `blackpill_f411ce.dts` (`adc1_in1_pa1`). Input range 0..VDDA (3.3 V) only.
