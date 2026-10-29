# qemu-trace-hooks

Trace hooks (traceTASK_SWITCHED_IN, traceQUEUE_SEND/RECEIVE) into a ring buffer, plus run-time stats from a hardware timer.

Runs on qemu `mps2-an385` (Cortex-M3, FreeRTOS ARM_CM3 port, board layer `toolchain/freertos/qemu/`); no board needed.
Features: freertos, trace, run-time-stats, qemu. The program checks itself and ends with `RESULT: PASS` (qemu exit 0) or `RESULT: FAIL` (exit 1).

## Run

```sh
scripts/app run qemu-trace-hooks              # build + run in qemu
scripts/app examples test qemu-trace-hooks    # the same, as a test (also runs test.sh if present)
```

## Output (qemu, 2026-10-29)

```
qemu-trace-hooks: trace macros and run-time stats
run-time stats (task, us, %):
report         	524		<1%
IDLE           	0		<1%
Tmr Svc        	2534		<1%
pong           	1359		<1%
ping           	1788		<1%
busy           	247158		97%
last context switches (task @ cycle):
  busy     @ 6324841
  busy     @ 6325121
  busy     @ 6325429
  busy     @ 6325752
  busy     @ 6326170
  report   @ 6327842
switches 21954, queue sends 100, receives 100
CHECK ok  : traceTASK_SWITCHED_IN recorded the context switches
CHECK ok  : traceQUEUE_SEND/RECEIVE counted every message
switch-ins: ping 201, pong 101
CHECK ok  : switch-ins into ping and pong seen by the hook
CHECK ok  : run-time stats list the tasks
CHECK ok  : the busy task used more CPU than ping
RESULT: PASS (5 checks)
```

On a board: the code uses only the kernel API plus `board.h` (UART log, a timer interrupt, a software interrupt); port the three board calls to run it on an STM32.
