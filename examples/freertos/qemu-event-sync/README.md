# qemu-event-sync

Event groups: three tasks meet at a barrier (xEventGroupSync) for 5 rounds; wait-any/wait-all with timeout; bits set from an ISR.

Runs on qemu `mps2-an385` (Cortex-M3, FreeRTOS ARM_CM3 port, board layer `toolchain/freertos/qemu/`); no board needed.
Features: freertos, event-groups, isr, qemu. The program checks itself and ends with `RESULT: PASS` (qemu exit 0) or `RESULT: FAIL` (exit 1).

## Run

```sh
scripts/app run qemu-event-sync              # build + run in qemu
scripts/app examples test qemu-event-sync    # the same, as a test (also runs test.sh if present)
```

## Output (qemu, 2026-10-29)

```
qemu-event-sync: barrier, wait-any/all, ISR bits
CHECK ok  : wait-all: all three parties finished
CHECK ok  : barrier: nobody ran ahead by a round
CHECK ok  : wait-any on an unset bit times out
CHECK ok  : bit set from an ISR woke the waiting task
CHECK ok  : clear-on-exit cleared the ISR bit
RESULT: PASS (5 checks)
```

On a board: the code uses only the kernel API plus `board.h` (UART log, a timer interrupt, a software interrupt); port the three board calls to run it on an STM32.
