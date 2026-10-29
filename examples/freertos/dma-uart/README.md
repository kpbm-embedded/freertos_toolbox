# freertos / dma-uart

Console server task sends USART1 TX by DMA; producers feed it through a queue.

**Status: builds; untested on hardware.**

Two producers (`fast` 250 ms, `slow` 1 s) format lines into a queue of 64-byte messages. The `server` task owns USART1 TX: it copies a line
into its DMA buffer, calls `HAL_UART_Transmit_DMA` and blocks on `ulTaskNotifyTake`. `HAL_UART_TxCpltCallback` (USART1 TC interrupt, priority 6,
after the DMA TC interrupt) gives the notification. The CPU is free during each transfer. Nothing else prints, because a
blocking `HAL_UART_Transmit` would collide with a running DMA transfer (HAL_BUSY).

Expected output:
```
freertos dma-uart on blackpill-f411: USART1 TX by DMA (64-byte lines, queue 8)
[fast] line 0, tick 0
[slow] line 0, free heap 26560 B
```

## Build
```
../../../scripts/app example dma-uart blackpill-f411   # from here (32_04_freertos/examples/freertos/dma-uart); or by hand (from 32_04_freertos/):
. toolchain/freertos/env.sh
cmake -S examples/freertos/dma-uart -B $FREERTOS_OUT/examples/dma-uart/blackpill-f411 -G Ninja -DBOARD=blackpill-f411
cmake --build $FREERTOS_OUT/examples/dma-uart/blackpill-f411 -j16        # -> dma-uart.{elf,bin,hex}
```
Through the category's `mcu` tool: `MCU_JOBS=4 bash templates/tools/check.sh --only dma-uart freertos`.
Flash: `st-flash --reset write dma-uart.bin 0x08000000` (or openocd/pyocd/probe-rs, see `toolchain/stm32/README.md`). Console: `python -m serial.tools.miniterm /dev/ttyUSB0 115200` with a USB-UART on PA9/PA10.

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

| Board | USART1_TX DMA | Source |
|---|---|---|
| blackpill-f411 | DMA2 Stream7, channel 4 | RM0383 DMA2 request mapping (not re-read; same as ST Cube F4 UART DMA examples) |
| weact-g431 | DMA1 Channel1, DMAMUX request `DMA_REQUEST_USART1_TX` (25) | `stm32g4xx_hal_dma.h`; any channel works with DMAMUX |
| mini-h743 | DMA1 Stream0, DMAMUX request `DMA_REQUEST_USART1_TX` (42) | `stm32h7xx_hal_dma.h`; buffers in AXI SRAM (DMA1 can reach it), D-cache not enabled by this board layer |
| bluepill-f103 | DMA1 Channel4 | RM0008 DMA1 request table (not re-read) |
