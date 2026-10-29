# 32_04_freertos - FreeRTOS on STM32 and qemu (freertos-box)

FreeRTOS-Kernel V11.3.2 on STM32 boards (BlackPill F411, WeAct G431, mini H743, BluePill F103), built with the STM32Cube HALs,
the Arm GNU toolchain, cmake and ninja, and on qemu `mps2-an385` (Cortex-M3) for tests without a board. Use it to build,
unit-test, run (qemu), flash and monitor FreeRTOS apps; 33 examples, 13 templates (C, Rust and Zig tasks; a host-side Bend check).

Status: **ready** (2026-10-29): 16 qemu examples run and pass, 16 board examples build (63 board builds), 11 templates
instantiate and build/run/test, smoke 24/24, doctor clean. **Untested on hardware** (no board attached): flash, monitor and
the board examples are build-checked only.
**Self-contained:** it uses only this folder (toolchains installed from `config/pins.tsv`), plus its build output.

## What's included

| Component | Version | Notes |
|---|---|---|
| FreeRTOS-Kernel | V11.3.2 (commit 640d0c4) | ports GCC/ARM_CM4F, ARM_CM7/r0p1, ARM_CM3; heap_4 |
| FreeRTOS+CLI | 202411.00 | `toolchain/freertos/plus-cli` (FreeRTOS_CLI.c/.h from the FreeRTOS release) |
| qemu board layer | this ID | `toolchain/freertos/qemu`: mps2-an385 startup, linker script, UART/timer/semihosting, FreeRTOSConfig, `freertos_qemu()` |
| xPack QEMU Arm | 9.2.4-1 | `toolchain/qemu/qemu-arm` (`qemu-system-arm -M mps2-an385`) |
| STM32Cube packs | F4, G4, H7, F1 | `toolchain/stm32` (F4 v1.28.3, F1 v1.8.7, G4 v1.6.3, H7 v1.13.0) |
| Arm GNU Toolchain | 15.2.rel1 | `toolchain/tools/toolchains/arm-gnu-toolchain` |
| cmake / ninja | 4.4 / 1.13 | host layer `toolchain/host` |
| Flash tools | st-flash 1.9.0, xPack OpenOCD 0.12.0, pyocd 0.46.0, probe-rs 0.32.0 | `toolchain/tools` |
| GoogleTest | 1.18.0 | host unit tests, `toolchain/clion` |
| App / examples / templates | `freertos-blinky-f411`; 16 board + 14 qemu examples; 11 templates | tables below |
| Yocto buildtools-extended | 6.0.4 | host gcc/g++/make/git/python for host tests (`toolchain/host`) |

Every downloaded component is pinned in `config/pins.tsv` (sha256 of the archive, or the git commit) and installed by
`scripts/setup` (`scripts/app bootstrap` = the `core` group). Groups: `core` (buildtools, cmake, ninja, Arm GCC, kernel,
FreeRTOS+CLI, GoogleTest, qemu), `stm32` (the four Cube packs, HAL/CMSIS/USB-device parts only, ~1 GB), `flash` (xPack
OpenOCD, probe-rs). Not in the pins: st-flash (built from source, `toolchain/tools/README.md`) and pyocd (a venv); doctor
only warns about them.

## Quick start

```sh
cd $BOX/30_39_tools/32_embedded/32_04_freertos   # or a fresh clone of freertos-box
scripts/app bootstrap                                   # core pins (~800 MB unpacked; downloads cached in cache/downloads)
scripts/setup install stm32                             # STM32Cube packs, only for the board examples
scripts/app doctor
scripts/app run qemu-queue-pipeline                     # an example on qemu, no board needed
scripts/app test --all                                  # smoke + all examples + all templates + app host tests
scripts/app help
scripts/app build freertos-blinky-f411 Release          # -> $FREERTOS_OUT/apps/.../freertos-blinky-f411.{elf,bin,hex}
scripts/app test  freertos-blinky-f411                  # host unit tests
scripts/app example blink                               # example on the default board
scripts/app flash freertos-blinky-f411 Debug            # with an ST-Link attached
```

## Commands

`scripts/app <command> [args]`; `<app>` is a folder under `apps/` or a path.

| Command | What it does | Example |
|---|---|---|
| `bootstrap` | install the `core` pins (`scripts/setup install core`) | `scripts/app bootstrap` |
| `setup <cmd> [sel]` | `scripts/setup`: `list`, `check`, `install`, `fetch`, `latest` for groups `core`, `stm32`, `flash`, `all` or one pin | `scripts/app setup install stm32` |
| `doctor` | core pins installed and tools run (stm32/flash parts only warn) | `scripts/app doctor` |
| `update [--check]` | `--check`: newest upstream tags of the pins; without: how to move a pin | `scripts/app update --check` |
| `build <app> [Debug\|Release]` | build to `$FREERTOS_OUT/apps/<app>/cmake-build-<profile>/<app>.{elf,bin,hex}` | `scripts/app build freertos-blinky-f411 Release` |
| `test <app>` | host unit tests (`tests/`, GoogleTest, CTest) | `scripts/app test freertos-blinky-f411` |
| `test [--all]` | `tests/test_smoke.sh`; `--all`: + `examples test`, `templates test`, app host tests | `scripts/app test --all` |
| `flash <app> [Debug\|Release] [st-flash\|openocd\|pyocd\|probe-rs]` | flash (default `st-flash`) | `scripts/app flash freertos-blinky-f411 Debug openocd` |
| `example <example> [board]` | build `examples/freertos/<example>` to `$FREERTOS_OUT/examples/<example>/<board>/` (default: first target in example.toml) | `scripts/app example mutex weact-g431` |
| `run <example\|dir>` | build a qemu example or app and run it in qemu; pass = `RESULT: PASS` and exit 0 | `scripts/app run qemu-cli` |
| `qemu <elf> [timeout]` | run an mps2-an385 ELF in qemu | `scripts/app qemu build/x.elf` |
| `examples list\|build\|test [ex...]` | qemu examples: build + run (+ `test.sh`); board examples: build every target | `scripts/app examples test` |
| `new <kind> <name> [--dir D]` | app from `templates/<kind>` into `apps/<name>` (or D/<name>) | `scripts/app new qemu-app demo` |
| `templates list\|test [kind...]` | instantiate each template in a temp dir, build, run (qemu) and host-test it | `scripts/app templates test` |
| `monitor [port] [baud]` | serial console (default `/dev/ttyUSB0` 115200) | `scripts/app monitor /dev/ttyUSB0 115200` |
| `where` | key paths: ID root, toolchain, kernel, build output, apps, examples | `scripts/app where` |
| `help` | command overview (also `-h`, `--help`) | `scripts/app help` |

Usage errors (missing app/example, a profile other than Debug/Release, unknown command or probe, a bad name) exit 2.
From anywhere: `box run freertos <command> ...` runs `scripts/app` with the same arguments.

## Examples

`examples/freertos/<ex>/` (`example.toml`, `README.md`, sources). qemu examples check themselves and print
`RESULT: PASS`; `scripts/app examples test` runs them (about 10 s for all 14). Board examples are build-checked.

| Example | Target | Shows |
|---|---|---|
| `qemu-queue-pipeline` | qemu | producer -> filter -> consumer over queues; blocking on a full queue, checksum |
| `qemu-semaphores` | qemu | binary semaphore from a timer ISR; counting semaphore as a 3-slot resource pool |
| `qemu-event-sync` | qemu | event-group barrier (xEventGroupSync), wait-any/all with timeout, bits from an ISR |
| `qemu-stream-buffer` | qemu | stream buffer from a 2 kHz ISR to a task, trigger level, byte-exact check |
| `qemu-message-buffer` | qemu | variable-length messages; too-small receive buffer refused |
| `qemu-sw-timers` | qemu | auto-reload/one-shot timers, timer ID, reset, period change, pended call |
| `qemu-task-notify` | qemu | notifications as binary/counting semaphore, event bits, mailbox, indexed |
| `qemu-static-alloc` | qemu | static allocation only, no heap linked (`test.sh` checks the ELF symbols) |
| `qemu-heap-stats` | qemu | heap_4 fragmentation/coalescing via vPortGetHeapStats, failed allocation hook |
| `qemu-tickless-idle` | qemu | tickless idle: fewer SysTick interrupts than ticks, pre/post-sleep hooks |
| `qemu-cli` | qemu | FreeRTOS+CLI commands in a self-test; `-DCLI_INTERACTIVE=ON` for a UART shell |
| `qemu-trace-hooks` | qemu | traceTASK_SWITCHED_IN / traceQUEUE_* recorder, run-time stats from a hardware counter |
| `qemu-mutex-inherit` | qemu | priority inheritance checked while it happens; recursive mutex |
| `qemu-stack-overflow` | qemu | method-2 stack overflow detection, high-water marks |
| `qemu-rust-task` | qemu | a task written in Rust (no_std staticlib) talking to C tasks over queues |
| `qemu-zig-task` | qemu | the same task written in Zig (freestanding object) |
| `host-bend-check` | qemu + host | the statistics of qemu-rust-task recomputed by a host-side Bend 2 program (skips without 32_31_bend) |
| adc-read, blink, button-irq, dma-uart, event-groups, i2c-scan, low-power, mutex, pwm-fade, stack-overflow-hook, sw-timers, tasks-queue, timer, uart-cli, uart-hello, usb-cdc | blackpill-f411 (default), weact-g431, mini-h743, bluepill-f103 (button-irq: not on the BluePill) | board peripherals with FreeRTOS; build-checked |

All ISR-to-task paths in the board examples use task notifications or stream buffers at NVIC priority 6; in the qemu examples
ISRs use raw priority 0xE0 (`BOARD_IRQ_PRIO_LOW`, below `configMAX_SYSCALL_INTERRUPT_PRIORITY` 0xA0).

## Templates

`scripts/app new <kind> <name> [--dir D]` copies `templates/<kind>` (placeholders `@NAME@` in files, `__name__` in file
names) to `apps/<name>`; refuses to overwrite. `scripts/app templates test` checks all of them.

| Template | Targets | What you get |
|---|---|---|
| `qemu-app` | qemu | two tasks + queue, self-checking; `src/app_config.h` overrides the kernel config |
| `qemu-isr-deferred` | qemu | short ISR -> task notification + stream buffer -> task |
| `qemu-static` | qemu | static allocation only, no heap |
| `qemu-cli` | qemu | FreeRTOS+CLI console with a command table |
| `qemu-active-object` | qemu + host | task owning a state machine and event queue; FSM in `lib/` with GoogleTest tests |
| `qemu-trace` | qemu | trace hook + run-time stats skeleton |
| `qemu-tickless` | qemu | tickless idle with pre/post-sleep hooks |
| `qemu-supervisor` | qemu | task health supervisor (event-group check-ins, where a watchdog is fed) |
| `stm32-app` | 4 STM32 boards | LED + log tasks on the board layer (`scripts/app build`, `flash`) |
| `stm32-app-tests` | 4 STM32 boards + host | logic in `lib/` (moving average) with GoogleTest host tests |
| `dual-target` | qemu + blackpill-f411 | one app, `platform/{qemu,stm32}.c`: test on qemu, deploy to the board |
| `qemu-rust-task` | qemu | FreeRTOS app with one Rust task (C shims for the kernel macros) |
| `qemu-zig-task` | qemu | FreeRTOS app with one Zig task |

## Rust, Zig, Bend

- **Rust, Zig: supported** as FreeRTOS tasks via FFI (`toolchain/freertos/qemu/langs.cmake`). The compilers are reached
  in their own IDs, not copied: Rust in `32_17_embedded_rust` (`$BOX_EMBEDDED_RUST`, else the sibling folder; rustc
  1.99.1, `thumbv7m-none-eabi`), Zig in `32_24_microzig` (`$BOX_MICROZIG`; Zig 0.17.0). Kernel macros (`xQueueSend`,
  `pdMS_TO_TICKS`, ...) get real C functions in `src/rtos_shim.c`. Without the ID, `needs.sh` exits 3 and the example or
  template is reported as SKIP.
- **Bend 2: host side only** (`host-bend-check` example): Bend has no MCU/RTOS target (its C output needs a hosted
  POSIX system with pthreads), so it is a host reference for task logic, compared with the FreeRTOS firmware on qemu.
  Reached in `32_31_bend` (`$BOX_BEND`); missing on 2026-10-29, so the example is skipped (exit 3), Bend file unchecked.

`mcu new freertos <name>` (tool `32_01_mcu`) also creates a minimal board app in `apps/`.

## Configuration and environment variables

| Variable | Default | Meaning |
|---|---|---|
| `FREERTOS_OUT` | `$BOX_V3/30_39_tools/32_embedded/32_04_freertos/build` | build output base |
| `JOBS` | `nproc`, at most 8 | parallel build jobs |
| `QEMU_TIMEOUT` | 60 | seconds before a qemu run is stopped |
| `SETUP_CACHE` | `cache/downloads` | download cache of `scripts/setup` (point it at another clone's cache to avoid downloads) |
| `FREERTOS_KERNEL` | set by `env.sh` | kernel source path |
| `STM32_CUBE_F4` (and `_F1`, `_G4`, `_H7`) | set by `env.sh` | Cube pack paths |

## Where files go

| What | Where |
|---|---|
| Source | this ID: `apps/`, `examples/freertos/` |
| Kernel, board layers, tools | `toolchain/freertos/` (kernel, `common/` shared config, `qemu/` board layer, `plus-cli/`), `toolchain/stm32/` (Cube packs, `boards/<board>/`), `toolchain/tools/`, `toolchain/host/`, `toolchain/qemu/` |
| Pins, downloads | `config/pins.tsv`; `cache/downloads/` (not in git) |
| Build output | `$FREERTOS_OUT` = `$BOX_OUT/30_39_tools/32_embedded/32_04_freertos/build/` (standalone or `BOX_INTEGRATION=0`: `build/` here): `apps/<app>/cmake-build-<profile>/`, `apps/<app>/qemu/`, `examples/<example>/<board>/` (+ `qemu.log`), `templates/` |
| Logs | in the build directories (ninja output) |

## Native mode

No native file exists for this ID, so `box native enable` does not apply. `scripts/app` sources the toolchain itself;
`box run freertos <command>` reaches it from anywhere. To use the compilers directly, enter the environment per shell (bash or zsh):

```sh
source $BOX/30_39_tools/32_embedded/32_04_freertos/toolchain/freertos/env.sh
arm-none-eabi-gcc --version; echo $FREERTOS_KERNEL $STM32_CUBE_F4
```

`env.sh` resolves its physical path, sources `../host/env.sh` and `../stm32/env.sh` (which sources `../tools/env.sh`) and sets `FREERTOS_KERNEL`.

## Tests

```sh
scripts/app test             # tests/test_smoke.sh: help, usage errors, pins format, setup dry run, doctor, template round trip on qemu
scripts/app test --all       # + examples test (14 qemu runs, 63 board builds), templates test (11), app host tests
```

Last run (2026-10-29): smoke 24/24; examples 79/79 (16 qemu PASS + 63 board builds); templates 13/13; app host tests 3/3;
doctor ok. Fresh clone (`BOX_INTEGRATION=0`, core pins + Cube F4 reinstalled from the pins): doctor ok, smoke 24/24,
examples 32 ok + 47 skipped (F1/G4/H7 packs not installed there), templates 13/13. Nothing ran on hardware.

## Git and a fresh clone

Repo `freertos-box`. Only sources are committed (`.gitignore`): toolchains, Cube packs, the kernel, qemu, GoogleTest,
downloads and build output are not. A clone runs `scripts/app bootstrap` (and `scripts/setup install stm32` for the board
examples) and gets the same pinned files. CLion's `.idea/cmake.xml` (absolute paths of one machine) is not committed;
open the app in CLion and set the toolchain environment from `toolchain/freertos/env.sh`. No symlinks into other IDs here.

## Troubleshooting

| Symptom | Fix |
|---|---|
| `STM32_CUBE_<F> not set` | the Cube pack is missing for that family; F4, G4, H7, F1 are present now (G4/H7/F1 were added 2026-10-24); source `env.sh` |
| `mcu` used its own stm32/host instead of this ID's | `env.sh` files resolve physically (`cd -P`); source this ID's `env.sh` |
| Doctor: "is not the bundled toolchain" | fixed; the Arm toolchain inside this ID's `toolchain/tools/toolchains` is accepted |
| Editing a file under `toolchain/` changes `mcu` too | hardlinked copies: check `stat -c %h <file>`; if above 1, write a new file and `mv` it over |
| Flash: probe permission denied | install the udev rules (needs sudo); see `32_01_mcu/toolchain/tools/udev` |
| `app build` with no app | usage error, exit 2 |

## Limitations / needs you

- No board attached: flash and run on hardware untested. The qemu examples run the real kernel and port (ARM_CM3) on an
  emulated Cortex-M3; timing checks there use qemu's virtual clock (a heavily loaded host can stretch them).
- st-flash (built from source) and pyocd (venv) are not in the pins; a fresh clone has OpenOCD and probe-rs from `flash`.
- udev rules need sudo once. Open items: `$BOX/00_09_meta/02_logs/2026_10_25_open_items.md`.
- Cosmetic: old DWARF/`__FILE__` path strings in a few third-party binaries.

## Layout

| Path | What |
|---|---|
| `toolchain/freertos/` | `env.sh`, `FreeRTOS-Kernel/`, `common/` (shared `FreeRTOSConfig.h`, `rtos_glue`, `freertos.cmake`), `qemu/` (mps2-an385 layer), `plus-cli/`, `examples/blinky-f411-freertos`, `sdk.sh` (the `mcu` plugin) |
| `toolchain/qemu/` | xPack QEMU Arm (`qemu-arm` -> `xpack-qemu-arm-9.2.4-1`) |
| `config/pins.tsv`, `scripts/setup` | pinned components and their installer |
| `templates/<kind>/` | app templates (`template.toml`, `README.md`, sources) |
| `tests/test_smoke.sh` | launcher smoke test |
| `toolchain/stm32/` | STM32Cube packs, `boards/<board>/`, `sdk-common.sh` (build/flash/doctor helpers), `env.sh` |
| `toolchain/tools/` | Arm GNU toolchain, st-flash, xPack OpenOCD, pyocd (venv), probe-rs |
| `toolchain/host/` | host layer: Yocto buildtools, cmake, ninja, uv, CPython, private libs |
| `toolchain/clion/` | `cmake/McuHostTests.cmake` + GoogleTest for host unit tests |
| `apps/<app>/` | your applications (CLion-ready: `.idea/`, `.run/`) |
| `examples/freertos/<ex>/` | the examples |

`toolchain/*` started as hardlinked copies of the `mcu` versions; files that needed their own paths (env.sh files,
`stm32/sdk-common.sh`, `freertos/sdk.sh`, READMEs) are separate inodes. The host/tools READMEs describe `mcu` commands because the copies came from there.
The `mcu` tool reaches this ID by symlink (`tool/freertos`), so `mcu build freertos blackpill-f411` uses this ID's toolchain; nothing here depends on `mcu`.

## Scripts

- `scripts/app` - bootstrap/doctor/update, build, test, run (qemu), flash (st-flash/openocd/pyocd/probe-rs) and monitor apps;
  examples and templates; `where`. `scripts/app help` lists the commands; exit codes: 0 ok, 1 failure, 2 usage error.
- `scripts/setup` - install/check/fetch the pins of `config/pins.tsv` (sha256 or git commit verified; never overwrites).
- `tests/test_smoke.sh` - smoke test (`scripts/app test`).
  Described in `.box.toml` (`[tool]` + `[scripts.app]`); `box describe freertos` shows it.

## Links

- [NOTES.md](NOTES.md), `toolchain/freertos/` (kernel), sibling tool `32_01_mcu`
- Reports in `$BOX/00_09_meta/02_logs/`: `2026_10_24_mcu_plugins_C_report.md`, `2026_10_24_embedded_extras_report.md`, `2026_10_25_readability_embedded_core_report.md`
