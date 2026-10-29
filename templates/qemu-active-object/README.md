# @NAME@

Active object: a task that owns a state machine and an event queue.

The state machine (`lib/fsm.c`) has no RTOS calls, so `tests/` checks it on the PC with GoogleTest; the firmware (`src/main.c`) runs it in a task on qemu.

## Build and run

```sh
scripts/app test apps/@NAME@   # host unit tests (lib/)
scripts/app run apps/@NAME@    # firmware on qemu
```

Made from the freertos-box template `qemu-active-object` (`scripts/app new qemu-active-object @NAME@`).
