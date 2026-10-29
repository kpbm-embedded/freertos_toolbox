# @NAME@

Task supervisor (software watchdog) with event groups.

Each worker sets its bit; the supervisor clears all bits once per period and feeds the hardware watchdog only when all were set.

## Build and run

```sh
scripts/app run apps/@NAME@
```

Made from the freertos-box template `qemu-supervisor` (`scripts/app new qemu-supervisor @NAME@`).
