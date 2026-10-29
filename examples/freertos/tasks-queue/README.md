# freertos / tasks-queue

Producer/consumer tasks over a FreeRTOS queue, with overflow accounting.

**Status: builds; untested on hardware.**

A 100 ms producer feeds an 8-slot queue; a 300 ms consumer drains it, so the queue fills and the producer logs dropped messages.

Expected log:
```
[    300] consumer: seq=0 sent@0 waiting=3
[   1100] producer: queue full, 1 dropped so far
```

## Build
```
../../../scripts/app example tasks-queue blackpill-f411   # from here; or by hand (from 32_04_freertos/):
. toolchain/freertos/env.sh
cmake -S examples/freertos/tasks-queue -B $FREERTOS_OUT/examples/tasks-queue/blackpill-f411 -G Ninja -DBOARD=blackpill-f411
cmake --build $FREERTOS_OUT/examples/tasks-queue/blackpill-f411 -j16        # -> tasks-queue.{elf,bin,hex}
```
Flash: `st-flash --reset write tasks-queue.bin 0x08000000` (or openocd/pyocd/probe-rs, see `toolchain/stm32/README.md`). Console: `python -m serial.tools.miniterm /dev/ttyUSB0 115200` with a USB-UART on PA9/PA10.

Targets: blackpill-f411, weact-g431, mini-h743, bluepill-f103 (select with `-DBOARD=`).

Pin and clock sources: the `board.cmake` of each board in `toolchain/stm32/boards/<board>/` cites them (cross-checked with the Zephyr board DTS in this
workspace; vendor schematics not re-read). Kernel: FreeRTOS-Kernel V11.3.2, port GCC/ARM_CM4F (F411, G431), ARM_CM7/r0p1 (H743), ARM_CM3 (F103), heap_4 (heap 32/16/64/8 KiB); shared config in `toolchain/freertos/common/FreeRTOSConfig.h`
(1 kHz tick, 4 priority bits, syscall ceiling 5). HAL timebase: DWT before the scheduler, RTOS tick after (`common/rtos_glue.c`).
Pins: BlackPill F411 LED PC13 (active low), console PA9 TX / PA10 RX; WeAct G431 LED PC6 (active high), mini H743 LED PE3 (active high), BluePill F103 LED PC13 (active low); console USART1 PA9 / PA10 on all, 115200 8N1.
