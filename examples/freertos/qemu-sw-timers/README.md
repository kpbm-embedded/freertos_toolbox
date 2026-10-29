# qemu-sw-timers

Software timers: auto-reload and one-shot, timer IDs, period change, reset, pended function calls.

Runs on qemu `mps2-an385` (Cortex-M3, FreeRTOS ARM_CM3 port, board layer `toolchain/freertos/qemu/`); no board needed.
Features: freertos, timers, qemu. The program checks itself and ends with `RESULT: PASS` (qemu exit 0) or `RESULT: FAIL` (exit 1).

## Run

```sh
scripts/app run qemu-sw-timers              # build + run in qemu
scripts/app examples test qemu-sw-timers    # the same, as a test (also runs test.sh if present)
```

## Output (qemu, 2026-10-29)

```
qemu-sw-timers: auto-reload, one-shot, ID, period change, pend call
periodic fired 20 times in ~205 ms; one-shot fired 1 time(s) at +80 ms
CHECK ok  : auto-reload 10 ms timer fired about 20 times
CHECK ok  : one-shot fired exactly once
CHECK ok  : reset moved the one-shot to ~80 ms
CHECK ok  : one-shot is dormant after firing
CHECK ok  : changed period (25 ms) gives about 4 callbacks in 105 ms
CHECK ok  : pended function ran in the timer task
RESULT: PASS (6 checks)
```

On a board: the code uses only the kernel API plus `board.h` (UART log, a timer interrupt, a software interrupt); port the three board calls to run it on an STM32.
