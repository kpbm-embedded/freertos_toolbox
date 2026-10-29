# 32_04_freertos notes

## 2026_10_24
- Reorg wrap-up. `toolchain/freertos/env.sh`, `toolchain/stm32/env.sh` and `toolchain/tools/env.sh` resolve physical paths (`cd -P`). Before this, when `mcu` reached freertos through its `tool/freertos` symlink, it silently used mcu's host/tools/stm32.
- `toolchain/stm32/sdk-common.sh`: the doctor accepts an `arm-none-eabi-gcc` that resolves inside this ID's `toolchain/tools/toolchains`. It used a stale `*/mcu/tools/toolchains/*` pattern, which gave the false "is not the bundled toolchain" warning. `toolchain/freertos/sdk.sh` labels say `toolchain/stm32` (were `24_91_toolchain`).
- Repaired `toolchain/host/patchelf-0.18.0/patchelf` (ELF interpreter) and the `_sysconfigdata` prefixes in `toolchain/{host,tools}/python/*`.
- New `scripts/app` (build/test/flash/example/monitor, only this ID's toolchain). Output: $BOX_OUT/30_39_tools/32_embedded/32_04_freertos/build.
- New `toolchain/clion/` (hardlinked McuHostTests.cmake + GoogleTest) for the apps' host tests.
- `apps/freertos-blinky-f411`: CLion build dirs moved to idisk03 at this ID and the run configs use `../../scripts/app`. README rewritten. Example READMEs and CMake hints point at this ID's paths. New `README.md` for the ID.
- Verified: app Debug build (text 11700 B), host tests 3/3, `scripts/app example mutex`, `mcu build/selfcheck freertos`, `mcu doctor freertos` (only the optional dfu-util warning remains).
- 2026_10_24 toolchain host/tools/stm32/freertos post-reorg check (report: 00_09_meta/02_logs/2026_10_24_mcu_plugins_C_report.md). Fixed: buildtools `ldconfig` (static) loader sections re-pointed to this ID's sysroot (new inode). 7 stale uv interpreter-cache entries moved to Trash. All tools run via `toolchain/freertos/env.sh` (arm-none-eabi gcc/gdb, pyocd, probe-rs, xPack openocd with and without OPENOCD_SCRIPTS, st-flash). Fresh `scripts/app build freertos-blinky-f411` OK; build.ninja references only this ID. Text self-containment grep: empty. Cosmetic leftovers: DWARF/`__FILE__` old paths in patchelf, libusb, stlink and libudev.
- 2026_10_24 embedded extras (00_09_meta/02_logs/2026_10_24_embedded_extras_report.md): `toolchain/tools/static/lib/libusb-1.0.a` and `toolchain/tools/stlink/{bin/st-flash,st-info,st-util,lib/libstlink.so.1.9.0}` replaced by the rebuilt binaries as new inodes (link count 1; no paths into another ID, no old-reorg path strings). st-flash v1.9.0 resolves this ID's libstlink. Old copies in Trash.
- 2026_10_24 embedded extras (00_09_meta/02_logs/2026_10_24_embedded_extras_report.md): verification build `$BOX_OUT/30_39_tools/32_embedded/32_04_freertos/build/_fresh_mcuC` moved to Trash (logged).
- 2026_10_24 examples for all four boards: `toolchain/stm32/STM32Cube{F1,G4,H7}` were missing (only F4 was here, so every G431/H743/F103 build failed with "STM32_CUBE_<F> not set"); added as hardlinked copies (`cp -al`) of the mcu ID's packs, like F4. New examples blink, button-irq, uart-hello, timer, pwm-fade, adc-read, i2c-scan, usb-cdc, dma-uart, low-power (tickless idle); the six old ones now also target mini-h743 and bluepill-f103. All ISR -> task paths use task notifications or stream buffers at NVIC priority 6. Verified: `scripts/app example <ex> <board>` 63/63 OK, build.ninja files reference only this ID. Untested on hardware. README examples row updated.
- 2026_10_25 readability pass (report: 00_09_meta/02_logs/2026_10_25_readability_embedded_core_report.md): scripts/app split into functions, help text, usage errors exit 2 (`app build` without an app was silent); build/example succeed when any of .bin/.elf/.hex exists (example used to fail when one was missing).
- 2026_10_25 readability pass 2 (report: 00_09_meta/02_logs/2026_10_25_readability2_embedded_core_report.md): `scripts/app where`; missing example, a profile other than Debug/Release (build and flash; flash did not check it) are usage errors (exit 2). Descriptor commands += test, where. README: `box run freertos ...`, native n/a.

## 2026-10-29
- Prepared for GitHub as `freertos_toolbox` (git repo in this folder, `.gitignore` keeps only sources: no toolchains, Cube
  packs, kernel, qemu, GoogleTest, downloads, build output). Host paths removed from tracked files (READMEs use
  `$FREERTOS_OUT`, env.sh comments are relative, `mcu-project.json` / `.run/Debug.run.xml` relative or `$PROJECT_DIR$`);
  the app's `.idea/cmake.xml` (absolute paths of this machine) is not committed.
- Self-sufficient install: new `scripts/setup` + `config/pins.tsv` (sha256 of every archive, git commit of every
  checkout; never overwrites; cache in `cache/downloads`). Groups: core (buildtools 6.0.4, cmake 4.4.4, ninja 1.13.2,
  Arm GNU 15.2.rel1, FreeRTOS-Kernel V11.3.2 at 640d0c4 = the tag plus its release note, FreeRTOS+CLI 202411.00,
  GoogleTest 1.18.0, xPack QEMU Arm 9.2.4-1), stm32 (Cube F4/F1/G4/H7, sparse: Drivers + USB device library),
  flash (xPack OpenOCD 0.12.0-7, probe-rs 0.32.0). `scripts/app bootstrap|setup|doctor|update --check`.
  st-flash (source build) and pyocd (venv) stay outside the pins; doctor only warns.
- New qemu board layer `toolchain/freertos/qemu/` (mps2-an385, Cortex-M3, ARM_CM3 port): startup, linker script,
  CMSDK UART/timers, semihosting exit, shared FreeRTOSConfig.h with per-app `app_config.h` overrides, `freertos_qemu()`.
  Self-test protocol: `CHECK ok/FAIL` lines and `RESULT: PASS|FAIL`, qemu exit 0/1.
- 14 new examples (all run on qemu and check themselves): qemu-queue-pipeline, qemu-semaphores, qemu-event-sync,
  qemu-stream-buffer, qemu-message-buffer, qemu-sw-timers, qemu-task-notify, qemu-static-alloc (+ test.sh: no heap
  symbols in the ELF), qemu-heap-stats, qemu-tickless-idle, qemu-cli (FreeRTOS+CLI), qemu-trace-hooks,
  qemu-mutex-inherit, qemu-stack-overflow.
- 11 templates (new: `templates/`, `scripts/app new <kind> <name> [--dir D]`, `@NAME@`/`__name__`): qemu-app,
  qemu-isr-deferred, qemu-static, qemu-cli, qemu-active-object (FSM + GoogleTest), qemu-trace, qemu-tickless,
  qemu-supervisor, stm32-app, stm32-app-tests (GoogleTest), dual-target (qemu + STM32 from one source).
- scripts/app: commands bootstrap, setup, doctor, update, run, qemu, examples list|build|test, new,
  templates list|test, test [--all]; the toolchain env is sourced only where needed (help/setup/doctor work in a fresh
  clone); `example` takes the default board from example.toml; JOBS capped at 8; build dirs reconfigure with
  `--fresh` when the same name was configured from another folder. tests/test_smoke.sh (24 checks).
- Problems found and fixed on the way: `configUSE_STATS_FORMATTING_FUNCTIONS` must follow dynamic allocation (static-only
  build failed); trace example used a 64-entry ring that a busy task filled (now per-task counters); the mutex test
  counted MEDIUM while LOW was legitimately blocked; mailbox test left the value in index 0 (clear on exit);
  stale CMake caches when an app name moved (`--fresh`).
- Results: smoke 24/24; examples 77/77 (14 qemu PASS, 63 board builds); templates 11/11 (9 qemu runs, 2 GoogleTest
  suites 4/4 each, 4 boards x 2 STM32 templates + dual-target built); app host tests 3/3; doctor ok; shellcheck clean.
  `scripts/app test --all` 73 s. Nothing ran on hardware.
- Fresh-clone check (clone on idisk03, `cache/` linked to this one, `BOX_INTEGRATION=0`, output inside the clone):
  doctor before bootstrap fails as expected; `scripts/app bootstrap` reinstalled the whole core group from the pins
  (buildtools, cmake, ninja, Arm GNU, kernel, FreeRTOS+CLI, GoogleTest, qemu; 49 s, archives from the cache, kernel
  fetched from GitHub); `scripts/setup install stm32cube-f4` fetched the F4 pack sparse from GitHub (14 s); doctor ok;
  `test --all`: smoke 24/24, examples 30 ok (14 qemu + 16 F411 builds) and 47 skipped (F1/G4/H7 packs not installed
  in the clone), templates 11/11, app host tests 3/3. Not reinstalled in the clone: the F1/G4/H7 Cube packs and the
  flash group (OpenOCD, probe-rs). Clone moved to the Trash afterwards (logged).
- Addendum (Rust/Zig/BEAM): new `toolchain/freertos/qemu/langs.cmake` (rustc of 32_17_embedded_rust, Zig of
  32_24_microzig, reached via `$BOX_EMBEDDED_RUST` / `$BOX_MICROZIG` or the sibling folder, never copied). Examples
  qemu-rust-task and qemu-zig-task (a task in Rust/Zig computing statistics over a queue, C shims for kernel macros)
  and templates of the same names: all PASS on qemu; `needs.sh` exits 3 (SKIP) when the ID is missing, honoured by
  `examples test` and `templates test`. Linker script now defines `__exidx_start/end` (Zig objects pull the Arm
  unwinder). BEAM: not supported (AtomVM is its own VM image, not a FreeRTOS-Kernel task); see README.
- Correction (third language = Bend 2, not BEAM; no BEAM/AtomVM code had been added, only a README line, now replaced):
  example host-bend-check (a FreeRTOS task on qemu vs. a host-side Bend reference in `src/stats.bend`, compared by
  test.sh). 32_31_bend did not exist yet, so needs.sh skips it (exit 3); the C/qemu part was run by hand (PASS), the
  Bend file is unchecked. Bend has no MCU/RTOS target; no such claim is made.
