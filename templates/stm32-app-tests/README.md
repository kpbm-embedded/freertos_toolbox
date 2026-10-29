# @NAME@

STM32 FreeRTOS app with host unit tests.

Logic in `lib/` (no HAL, no RTOS) is tested on the PC (`tests/`, GoogleTest); `src/` is the firmware.

## Build and run

```sh
scripts/app test apps/@NAME@     # host unit tests
scripts/app build apps/@NAME@    # firmware (BlackPill F411)
```

Made from the freertos-box template `stm32-app-tests` (`scripts/app new stm32-app-tests @NAME@`).
