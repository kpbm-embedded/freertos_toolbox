# qemu-tickless-idle

Tickless idle: the kernel stops SysTick while all tasks block; far fewer tick interrupts than ticks, time stays right.

Runs on qemu `mps2-an385` (Cortex-M3, FreeRTOS ARM_CM3 port, board layer `toolchain/freertos/qemu/`); no board needed.
Features: freertos, tickless, low-power, qemu. The program checks itself and ends with `RESULT: PASS` (qemu exit 0) or `RESULT: FAIL` (exit 1).

## Run

```sh
scripts/app run qemu-tickless-idle              # build + run in qemu
scripts/app examples test qemu-tickless-idle    # the same, as a test (also runs test.sh if present)
```

## Output (qemu, 2026-10-29)

```
qemu-tickless-idle: tick suppression while idle
ticks 1000, SysTick interrupts 10, cycle-counter time 1001 ms, sleeps 11, wakeups 11
CHECK ok  : tick count advanced by ~1000 (10 x 100 ms)
CHECK ok  : far fewer SysTick interrupts than ticks: the tick was suppressed
CHECK ok  : pre/post sleep hooks ran in pairs
CHECK ok  : kernel time agrees with the hardware counter
RESULT: PASS (4 checks)
```

On a board: the code uses only the kernel API plus `board.h` (UART log, a timer interrupt, a software interrupt); port the three board calls to run it on an STM32.
