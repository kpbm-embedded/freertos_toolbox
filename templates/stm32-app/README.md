# @NAME@

FreeRTOS app for an STM32 board.

Board layer: `toolchain/stm32/boards/<board>` (clock, LED, console); kernel config: `toolchain/freertos/common/FreeRTOSConfig.h`. Needs the Cube packs (`scripts/app setup install stm32`). Flashing needs a probe (not tested here).

## Build and run

```sh
scripts/app build apps/@NAME@                     # BlackPill F411
cmake -S apps/@NAME@ -B /tmp/b -G Ninja -DBOARD=weact-g431 && cmake --build /tmp/b   # another board (inside env.sh)
scripts/app flash apps/@NAME@ Debug st-flash      # board attached
```

Made from the freertos-box template `stm32-app` (`scripts/app new stm32-app @NAME@`).
