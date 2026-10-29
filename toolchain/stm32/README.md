# stm32 - STM32Cube firmware (STMicroelectronics GitHub, BSD-3/Apache-2, no EULA tools)

This is 32_04_freertos's own copy of the stm32 layer (Cube packs, boards, helpers), used by FreeRTOS builds. The `mcu ...` commands below describe the category's `mcu` tool, which has its own stm32 copy.
| Pack | Tag |
|---|---|
| STM32CubeF4 | v1.28.3 |
| STM32CubeF1 | v1.8.7 |
| STM32CubeG4 | v1.6.3 |
| STM32CubeH7 | v1.13.0 |
Toolchain: bundled Arm GNU Toolchain 15.2.rel1 (../tools/toolchains/arm-gnu-toolchain), Ninja, cmake 4.4 (all from ../tools).
## Build
`../mcu build stm32 blackpill-f411 blinky-f411` -> `$MCU_OUT_BASE/stm32/blackpill-f411/blinky-f411/blinky-f411.{elf,bin,hex}`
or manually: `. env.sh; cmake -S examples/blinky-f411 -B build && cmake --build build`.
`examples/common/` holds toolchain file, HAL/CMSIS/startup CMake, own F411CE linker script, 100 MHz clock (HSE 25 MHz, PLL M25 N200 P2). BlackPill LED PC13 active low.
Result: text 3900, data 20, bss 2084 bytes.
## Flash (`../mcu flash stm32 blackpill-f411 blinky-f411 [st-flash|openocd|pyocd|probe-rs|dfu]`)
- st-flash: `st-flash --reset write blinky-f411.bin 0x08000000`
- openocd: `openocd -f interface/stlink.cfg -f target/stm32f4x.cfg -c "program blinky-f411.elf verify reset exit"`
- pyocd: `pyocd flash -t stm32f411ceux blinky-f411.elf`
- probe-rs: `probe-rs download --chip STM32F411CEUx blinky-f411.elf`
- DFU: hold BOOT0 while pressing reset, `dfu-util -a 0 -d 0483:df11 -s 0x08000000:leave -D blinky-f411.bin` (dfu-util not installed here).
`../mcu new stm32 <name>` copies the blinky into `$MCU_ROOT/projects/<name>`.
Not done: G431/H743 examples (packs F1/G4/H7 are cloned; same recipe applies).

## Self-contained
env.sh sources `../host/env.sh` then `../tools/env.sh` (shared Arm GNU toolchain 15.2, st-flash, openocd, pyocd, probe-rs). CMake/Ninja come from `host/` (the pip copies in `tools/venv` were removed). Cube packs are in this dir. Only glibc/sysbin utilities come from the OS.

## Templates, examples and the shared board layer
`boards/<board>/` (blackpill-f411, weact-g431, mini-h743, bluepill-f103) holds the clock tree, LED/button pins, linker parameters and HAL-config generation used by `templates/stm32/*` and `examples/stm32/*` (see `templates/README.md`, `examples/README.md`). Include `boards/board.cmake`, set `-DBOARD=<board>`, call `stm32_board(<tgt> [MODULES adc i2c ...])`. Pin/clock sources are cited in each `board.cmake`. Build+check everything: `templates/tools/check.sh stm32`.
