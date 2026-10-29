# qemu-static-alloc

Static allocation only (configSUPPORT_DYNAMIC_ALLOCATION 0, no heap linked): tasks, queue, mutex, event group, timer, stream buffer.

Runs on qemu `mps2-an385` (Cortex-M3, FreeRTOS ARM_CM3 port, board layer `toolchain/freertos/qemu/`); no board needed.
Features: freertos, static-allocation, qemu. The program checks itself and ends with `RESULT: PASS` (qemu exit 0) or `RESULT: FAIL` (exit 1).

## Run

```sh
scripts/app run qemu-static-alloc              # build + run in qemu
scripts/app examples test qemu-static-alloc    # the same, as a test (also runs test.sh if present)
```

`test.sh` checks the built ELF afterwards (see the file).

## Output (qemu, 2026-10-29)

```
qemu-static-alloc: no heap, all objects static
CHECK ok  : queue: 1..10 summed under the mutex
CHECK ok  : stream buffer carried the text
CHECK ok  : event group: producer bit and timer bit
CHECK ok  : static timer fired
CHECK ok  : static producer task suspended itself
RESULT: PASS (5 checks)
```

On a board: the code uses only the kernel API plus `board.h` (UART log, a timer interrupt, a software interrupt); port the three board calls to run it on an STM32.
