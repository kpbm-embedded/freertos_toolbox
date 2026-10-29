# @NAME@

FreeRTOS with static allocation only (no heap).

`freertos_qemu(... HEAP none)` links no heap_x.c; `pvPortMalloc` does not exist, so any dynamic create call fails at link time.

## Build and run

```sh
scripts/app run apps/@NAME@
arm-none-eabi-nm <build>/@NAME@.elf | grep -c pvPortMalloc   # 0
```

Made from the freertos-box template `qemu-static` (`scripts/app new qemu-static @NAME@`).
