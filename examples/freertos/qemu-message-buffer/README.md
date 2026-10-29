# qemu-message-buffer

Message buffers: variable-length messages between tasks, length kept per message, too-small receive buffer handled.

Runs on qemu `mps2-an385` (Cortex-M3, FreeRTOS ARM_CM3 port, board layer `toolchain/freertos/qemu/`); no board needed.
Features: freertos, message-buffer, qemu. The program checks itself and ends with `RESULT: PASS` (qemu exit 0) or `RESULT: FAIL` (exit 1).

## Run

```sh
scripts/app run qemu-message-buffer              # build + run in qemu
scripts/app examples test qemu-message-buffer    # the same, as a test (also runs test.sh if present)
```

## Output (qemu, 2026-10-29)

```
qemu-message-buffer: variable-length messages
50 of 50 messages intact, 44 too-small reads refused
CHECK ok  : every message arrived whole with its own length
CHECK ok  : receive into a too-small buffer returned 0
CHECK ok  : buffer empty at the end
CHECK ok  : all space free again
RESULT: PASS (4 checks)
```

On a board: the code uses only the kernel API plus `board.h` (UART log, a timer interrupt, a software interrupt); port the three board calls to run it on an STM32.
