# qemu-stream-buffer

Stream buffer from a 2 kHz timer ISR to a task with a trigger level; every byte checked in order.

Runs on qemu `mps2-an385` (Cortex-M3, FreeRTOS ARM_CM3 port, board layer `toolchain/freertos/qemu/`); no board needed.
Features: freertos, stream-buffer, isr, qemu. The program checks itself and ends with `RESULT: PASS` (qemu exit 0) or `RESULT: FAIL` (exit 1).

## Run

```sh
scripts/app run qemu-stream-buffer              # build + run in qemu
scripts/app examples test qemu-stream-buffer    # the same, as a test (also runs test.sh if present)
```

## Output (qemu, 2026-10-29)

```
qemu-stream-buffer: timer ISR -> stream buffer -> task
received 600 bytes in 39 wakeups, 0 bad, 0 dropped by the ISR
CHECK ok  : all bytes received
CHECK ok  : bytes in order, none lost or duplicated
CHECK ok  : buffer never overflowed
CHECK ok  : trigger level batched the bytes (far fewer wakeups than bytes)
RESULT: PASS (4 checks)
```

On a board: the code uses only the kernel API plus `board.h` (UART log, a timer interrupt, a software interrupt); port the three board calls to run it on an STM32.
