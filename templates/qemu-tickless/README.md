# @NAME@

Tickless idle skeleton.

Put the MCU-specific sleep entry/exit into `configPRE_SLEEP_PROCESSING` / `configPOST_SLEEP_PROCESSING` (src/app_config.h).

## Build and run

```sh
scripts/app run apps/@NAME@
```

Made from the freertos-box template `qemu-tickless` (`scripts/app new qemu-tickless @NAME@`).
