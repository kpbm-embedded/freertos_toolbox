# freertos / i2c-scan

I2C1 bus scan task (i2cdetect-style table every 5 s).

**Status: builds; untested on hardware.**

The `scan` task probes 0x08..0x77 with `HAL_I2C_IsDeviceReady` (1 try, 5 ms timeout), prints a table with `rtos_print` (mutex-protected
console) and sleeps 5 s with `vTaskDelay`. An LED task blinks at 2 Hz meanwhile. 100 kHz: F411/F103 via `ClockSpeed`;
G431 I2C1 kernel clock HSI 16 MHz, `TIMINGR` 0x00303D5B; H743 HSI 64 MHz, `TIMINGR` 0x30303D5B (PRESC 3 -> 16 MHz, same values).
The TIMINGR values are the usual CubeMX 16 MHz / 100 kHz settings, unverified on hardware. Fit 4.7 k pull-ups to 3.3 V.

Expected output:
```
     0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f
00:                         -- -- -- -- -- -- -- --
...
70: -- -- -- -- -- -- 76 --
[   1203] 1 device(s) on I2C1
```

## Build
```
../../../scripts/app example i2c-scan blackpill-f411   # from here (32_04_freertos/examples/freertos/i2c-scan); or by hand (from 32_04_freertos/):
. toolchain/freertos/env.sh
cmake -S examples/freertos/i2c-scan -B $FREERTOS_OUT/examples/i2c-scan/blackpill-f411 -G Ninja -DBOARD=blackpill-f411
cmake --build $FREERTOS_OUT/examples/i2c-scan/blackpill-f411 -j16        # -> i2c-scan.{elf,bin,hex}
```
Through the category's `mcu` tool: `MCU_JOBS=4 bash templates/tools/check.sh --only i2c-scan freertos`.
Flash: `st-flash --reset write i2c-scan.bin 0x08000000` (or openocd/pyocd/probe-rs, see `toolchain/stm32/README.md`). Console: `python -m serial.tools.miniterm /dev/ttyUSB0 115200` with a USB-UART on PA9/PA10.

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

| Board | SCL | SDA | Source |
|---|---|---|---|
| blackpill-f411 | PB8 (AF4) | PB9 (AF4) | Zephyr `blackpill_f411ce.dts` (`i2c1_scl_pb8`, `i2c1_sda_pb9`) |
| weact-g431 | PA15 (AF4) | PB7 (AF4) | Zephyr hal_stm32 pinctrl data (`modules/hal/stm32/dts/st/<family>/stm32<part>-pinctrl.dtsi`, generated from ST's pin database) (`i2c1_scl_pa15`, `i2c1_sda_pb7`). PB8 is avoided because it is shared with BOOT0 on the G431 (datasheet pinout, not re-read). Header availability unverified |
| mini-h743 | PB8 (AF4) | PB9 (AF4) | Zephyr `mini_stm32h743.dts` (`i2c1_scl_pb8`, `i2c1_sda_pb9`, the camera I2C) |
| bluepill-f103 | PB6 | PB7 | Zephyr hal_stm32 pinctrl data (`modules/hal/stm32/dts/st/<family>/stm32<part>-pinctrl.dtsi`, generated from ST's pin database) (`i2c1_scl_pb6`, `i2c1_sda_pb7`, I2C1_REMAP0) |
