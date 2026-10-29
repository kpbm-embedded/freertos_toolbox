# qemu-queue-pipeline

Three-stage pipeline (producer, filter, consumer) over queues; checks every item and the checksum.

Runs on qemu `mps2-an385` (Cortex-M3, FreeRTOS ARM_CM3 port, board layer `toolchain/freertos/qemu/`); no board needed.
Features: freertos, queue, qemu. The program checks itself and ends with `RESULT: PASS` (qemu exit 0) or `RESULT: FAIL` (exit 1).

## Run

```sh
scripts/app run qemu-queue-pipeline              # build + run in qemu
scripts/app examples test qemu-queue-pipeline    # the same, as a test (also runs test.sh if present)
```

## Output (qemu, 2026-10-29)

```
qemu-queue-pipeline: producer -> filter -> consumer
received 100 items, sum 30300 (want 30300), producer blocked 96 times
CHECK ok  : every even item arrived
CHECK ok  : checksum matches
CHECK ok  : items arrived in order
CHECK ok  : producer blocked on a full queue at least once
CHECK ok  : raw queue drained
RESULT: PASS (5 checks)
```

On a board: the code uses only the kernel API plus `board.h` (UART log, a timer interrupt, a software interrupt); port the three board calls to run it on an STM32.
