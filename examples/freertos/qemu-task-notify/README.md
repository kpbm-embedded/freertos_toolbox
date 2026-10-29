# qemu-task-notify

Direct-to-task notifications: binary/counting semaphore from an ISR, event bits, mailbox with/without overwrite, indexed notifications.

Runs on qemu `mps2-an385` (Cortex-M3, FreeRTOS ARM_CM3 port, board layer `toolchain/freertos/qemu/`); no board needed.
Features: freertos, task-notifications, isr, qemu. The program checks itself and ends with `RESULT: PASS` (qemu exit 0) or `RESULT: FAIL` (exit 1).

## Run

```sh
scripts/app run qemu-task-notify              # build + run in qemu
scripts/app examples test qemu-task-notify    # the same, as a test (also runs test.sh if present)
```

## Output (qemu, 2026-10-29)

```
qemu-task-notify: five uses of task notifications
CHECK ok  : 1 binary: notified from the ISR
CHECK ok  : 2 counting: three gives, three takes
CHECK ok  : 3 bits: 0x1 | 0x4 collected in one wait
CHECK ok  : 4 mailbox: no-overwrite refused while a value is pending
CHECK ok  : 4 mailbox: overwrite replaced the unread 111
CHECK ok  : 5 indexed: index 1 notified
CHECK ok  : 5 indexed: index 0 untouched
RESULT: PASS (7 checks)
```

On a board: the code uses only the kernel API plus `board.h` (UART log, a timer interrupt, a software interrupt); port the three board calls to run it on an STM32.
