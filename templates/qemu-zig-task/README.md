# @NAME@

FreeRTOS on qemu mps2-an385 with one task written in Zig (`src/task.zig`) next to C tasks.

The Zig code calls the kernel through `src/rtos_shim.c` (real C functions for the kernel's macros such as
`xQueueSend` and `pdMS_TO_TICKS`). The Zig compiler is not part of freertos-box: it is reached in
32_24_microzig (`$BOX_MICROZIG`, Zig 0.17.0);
`needs.sh` exits 3 (skip) when that ID is missing.

## Build and run

```sh
scripts/app run apps/@NAME@
```

Made from the freertos-box template `qemu-zig-task` (`scripts/app new qemu-zig-task @NAME@`).
