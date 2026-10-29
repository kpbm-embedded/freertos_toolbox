# freertos / usb-cdc

USB CDC-ACM virtual COM port echo (ST USB Device library) with an ISR -> stream buffer -> task path.

**Status: builds; untested on hardware.**

The Cube USB Device library (Core + CDC class from the board family's Cube pack, `usbd_conf.c` glue to HAL_PCD, `usbd_desc.c`)
enumerates as a CDC-ACM port. The CDC receive callback runs in the USB interrupt (priority 6): it only pushes the packet into a
stream buffer (`xStreamBufferSendFromISR`) and re-arms reception. The `echo` task sends the bytes back (waits up to 50 ms for the
previous IN packet, arms the endpoint inside a critical section); the `stats` task logs counts on USART1.
VID/PID 0483:5740 is ST's published VCP pair; use your own for a product.

USB clock per board: F411 PLLQ 48 MHz (`BOARD_USB48`: SYSCLK becomes 96 MHz); G431 PLLQ 48 MHz (board clock tree);
H743 HSI48 trimmed by the CRS on USB SOF; F103 PLL 72 MHz / 1.5. The F1 pack carries the older USB Device library V2.5.3
(16-bit sizes, no TransmitCplt callback); `usbd_conf.c` handles both.

Expected output:
A new `/dev/ttyACM*` appears; what you type into it comes back. On the UART console:
```
[   4000] usb: rx 12 B, echoed 12 B, dropped 0 B
```

## Build
```
../../../scripts/app example usb-cdc blackpill-f411   # from here (32_04_freertos/examples/freertos/usb-cdc); or by hand (from 32_04_freertos/):
. toolchain/freertos/env.sh
cmake -S examples/freertos/usb-cdc -B $FREERTOS_OUT/examples/usb-cdc/blackpill-f411 -G Ninja -DBOARD=blackpill-f411
cmake --build $FREERTOS_OUT/examples/usb-cdc/blackpill-f411 -j16        # -> usb-cdc.{elf,bin,hex}
```
Through the category's `mcu` tool: `MCU_JOBS=4 bash templates/tools/check.sh --only usb-cdc freertos`.
Flash: `st-flash --reset write usb-cdc.bin 0x08000000` (or openocd/pyocd/probe-rs, see `toolchain/stm32/README.md`). Console: `python -m serial.tools.miniterm /dev/ttyUSB0 115200` with a USB-UART on PA9/PA10.

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

| Board | USB peripheral | Pins | Source |
|---|---|---|---|
| blackpill-f411 | OTG_FS (AF10) | PA11 DM / PA12 DP, USB-C | Zephyr `blackpill_f411ce.dts` (`usb_otg_fs_dm_pa11`, `usb_otg_fs_dp_pa12`) |
| weact-g431 | USB FS device (FSDEV), internal D+ pull-up | PA11 / PA12, USB-C | Zephyr `weact_stm32g431_core.dts` (`&usb`: `usb_dm_pa11`, `usb_dp_pa12`) |
| mini-h743 | OTG2_FS (AF10), VBUS sensing off | PA11 / PA12, USB-C | Zephyr `mini_stm32h743.dts` (`usb_otg_fs_dm_pa11`, `usb_otg_fs_dp_pa12`) |
| bluepill-f103 | USB FS device (FSDEV) | PA11 / PA12, micro-USB | pinctrl `stm32f103c(8-b)tx` (`usb_dm_pa11`, `usb_dp_pa12`). The BluePill's external D+ pull-up (R10, often 10 k instead of 1.5 k on clones) is unverified; PA12 is driven low 10 ms at start so the host re-enumerates |

VBUS sensing is off on the OTG boards because PA9 (OTG VBUS) is the console TX in this board layer.
