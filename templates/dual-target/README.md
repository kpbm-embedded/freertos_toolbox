# @NAME@

One app, two targets: qemu for development and tests, an STM32 board for deployment.

`src/` uses only FreeRTOS and `platform/platform.h`; `platform/qemu.c` and `platform/stm32.c` implement it.

## Build and run

```sh
scripts/app run apps/@NAME@                      # qemu
cmake -S apps/@NAME@ -B /tmp/b -G Ninja -DBOARD=blackpill-f411 && cmake --build /tmp/b   # board (inside env.sh)
```

Made from the freertos-box template `dual-target` (`scripts/app new dual-target @NAME@`).
