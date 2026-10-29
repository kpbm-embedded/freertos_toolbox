# @NAME@

FreeRTOS on qemu mps2-an385 with one task written in Rust (`src/task.rs`) next to C tasks.

The Rust code calls the kernel through `src/rtos_shim.c` (real C functions for the kernel's macros such as
`xQueueSend` and `pdMS_TO_TICKS`). The Rust compiler is not part of freertos-box: it is reached in
32_17_embedded_rust (`$BOX_EMBEDDED_RUST`, rustc 1.99.1, thumbv7m-none-eabi);
`needs.sh` exits 3 (skip) when that ID is missing.

## Build and run

```sh
scripts/app run apps/@NAME@
```

Made from the freertos-box template `qemu-rust-task` (`scripts/app new qemu-rust-task @NAME@`).
