# qemu-semaphores

Binary semaphore given by a timer interrupt (ISR to task) and a counting semaphore guarding a pool of 3 resources.

Runs on qemu `mps2-an385` (Cortex-M3, FreeRTOS ARM_CM3 port, board layer `toolchain/freertos/qemu/`); no board needed.
Features: freertos, semaphore, isr, qemu. The program checks itself and ends with `RESULT: PASS` (qemu exit 0) or `RESULT: FAIL` (exit 1).

## Run

```sh
scripts/app run qemu-semaphores              # build + run in qemu
scripts/app examples test qemu-semaphores    # the same, as a test (also runs test.sh if present)
```

## Output (qemu, 2026-10-29)

```
qemu-semaphores: ISR -> binary semaphore, counting semaphore pool
timer ISR 60, handled 60; jobs 30, max channels in use 3
CHECK ok  : timer interrupt ran (>= 40 in 300 ms)
CHECK ok  : handler took the binary semaphore (never more than given)
CHECK ok  : all 30 jobs finished
CHECK ok  : pool limit held: at most 3 at once, and all 3 used
CHECK ok  : all tokens returned
RESULT: PASS (5 checks)
```

On a board: the code uses only the kernel API plus `board.h` (UART log, a timer interrupt, a software interrupt); port the three board calls to run it on an STM32.
