# @NAME@

Trace hooks and run-time statistics.

Run-time stats count microseconds from the 25 MHz TIMER0 of the board layer; on an STM32 use DWT->CYCCNT or a spare timer.

## Build and run

```sh
scripts/app run apps/@NAME@
```

Made from the freertos-box template `qemu-trace` (`scripts/app new qemu-trace @NAME@`).
