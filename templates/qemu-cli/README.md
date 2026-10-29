# @NAME@

FreeRTOS+CLI console (FreeRTOS_CLI.c from scripts/setup).

Self-test by default; configure with `-DCLI_INTERACTIVE=ON` for a shell on UART0 (qemu: the terminal).

## Build and run

```sh
scripts/app run apps/@NAME@
```

Made from the freertos-box template `qemu-cli` (`scripts/app new qemu-cli @NAME@`).
