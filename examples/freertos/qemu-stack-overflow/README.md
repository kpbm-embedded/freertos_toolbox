# qemu-stack-overflow

Stack overflow detection (configCHECK_FOR_STACK_OVERFLOW 2): a task overruns its stack, the hook names it; high-water marks.

Runs on qemu `mps2-an385` (Cortex-M3, FreeRTOS ARM_CM3 port, board layer `toolchain/freertos/qemu/`); no board needed.
Features: freertos, stack-overflow, debug, qemu. The program checks itself and ends with `RESULT: PASS` (qemu exit 0) or `RESULT: FAIL` (exit 1).

## Run

```sh
scripts/app run qemu-stack-overflow              # build + run in qemu
scripts/app examples test qemu-stack-overflow    # the same, as a test (also runs test.sh if present)
```

## Output (qemu, 2026-10-29)

```
qemu-stack-overflow: method 2 detection
checker stack high-water mark: 502 words free of 512
CHECK ok  : high-water mark of a healthy task is plausible
stack overflow hook: task 'greedy'
CHECK ok  : the overflow came from greedy (the expected one)
CHECK ok  : the hook named the overflowing task
RESULT: PASS (3 checks)
```

On a board: the code uses only the kernel API plus `board.h` (UART log, a timer interrupt, a software interrupt); port the three board calls to run it on an STM32.
