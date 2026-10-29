# @NAME@

FreeRTOS app for qemu mps2-an385 (Cortex-M3): two tasks and a queue.

Board layer: `toolchain/freertos/qemu/` (UART0 console via `log_printf`, timers, semihosting exit). Kernel settings: `src/app_config.h` overrides the shared FreeRTOSConfig.h.

## Build and run

```sh
scripts/app run apps/@NAME@          # build + run in qemu, passes on RESULT: PASS
scripts/app build apps/@NAME@        # only build (ELF/bin/hex, map)
```

Made from the freertos-box template `qemu-app` (`scripts/app new qemu-app @NAME@`).
