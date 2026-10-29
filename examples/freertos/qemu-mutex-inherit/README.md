# qemu-mutex-inherit

Mutex priority inheritance checked live (low task raised to high while it holds the mutex) and recursive mutex depth.

Runs on qemu `mps2-an385` (Cortex-M3, FreeRTOS ARM_CM3 port, board layer `toolchain/freertos/qemu/`); no board needed.
Features: freertos, mutex, priority-inheritance, qemu. The program checks itself and ends with `RESULT: PASS` (qemu exit 0) or `RESULT: FAIL` (exit 1).

## Run

```sh
scripts/app run qemu-mutex-inherit              # build + run in qemu
scripts/app examples test qemu-mutex-inherit    # the same, as a test (also runs test.sh if present)
```

## Output (qemu, 2026-10-29)

```
qemu-mutex-inherit: priority inheritance and recursive mutex
CHECK ok  : LOW inherited HIGH's priority while holding the mutex
CHECK ok  : LOW is back at its own priority after the give
CHECK ok  : MEDIUM did not run while LOW held the mutex (no inversion)
CHECK ok  : recursive mutex: still held after 2 of 3 gives
CHECK ok  : recursive mutex: free after the 3rd give
RESULT: PASS (5 checks)
```

On a board: the code uses only the kernel API plus `board.h` (UART log, a timer interrupt, a software interrupt); port the three board calls to run it on an STM32.
