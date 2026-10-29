# mcu/host - shared host layer (no OS packages)
Source `host/env.sh` first (every `<sdk>/env.sh` does). It gives PATH entries for:

| Component | Version | Source (sha256 checked) |
|---|---|---|
| Yocto buildtools-extended (git, make, gcc/g++ 15.3, tar, patch, wget, m4, pkg-config) | 6.0.4 | Yocto release `.sh` + `.sha256sum` in `dl/` |
| CMake / ninja / uv | 4.4.4 / 1.13.2 / 0.13.5 | Kitware, ninja-build, astral release tarballs in `dl/` |
| CPython (uv-managed, `python/`) | 3.12.15, 3.13.17, 3.8.20 (3.8 only for the Arm `arm-none-eabi-gdb-py`) | `uv python install --no-bin` |
| zstd | 1.5.7 (`eb33e51f...6fa3`, matches the release `.sha256`) | facebook/zstd release tarball, built with buildtools gcc |
| zlib | 1.3.2 (`bb329a0a...d16`, matches zlib.net) | madler/zlib release |
| ncurses (widec, `--with-termlib`, versioned syms) | 6.5 (`136d91bc...1cc6`) | GNU ftp; `libtinfo.so.6` is a symlink to `libtinfow.so.6` |
| libudev (eudev, built with gperf 3.1 only as a build helper) | 3.2.14 (`8da43191...006f`) | eudev-project release |
| libusb | 1.0.30 (`fea36f34...dbaf`), `--disable-udev`; shared in `lib/`, static+PIC in `static/` | libusb release |
| patchelf | 0.18.0 (`64de10e4...86e7`) | NixOS/patchelf release, `bin/patchelf` |
| libexpat, libgmp, libmpfr, liblzma | copied from the buildtools sysroot | for gdb builds |

## Private libraries: RUNPATH, not LD_LIBRARY_PATH
Prebuilt vendor binaries (Arm/Zephyr/ESP/Bouffalo/Pico toolchains, openocd, picotool, wlink, sdar, vnproch55x...) were linked against OS libs
(libzstd, libz, libusb, libudev, libncursesw, libexpat, libgmp, libpython3.X...). `host/patch-rpath.sh <dir|file>...` adds an
`$ORIGIN`-relative RUNPATH to `host/lib` (or `host/python/cpython-X.Y-*/lib` for libpythonX.Y) wherever a library resolves from the OS.
It is idempotent; rerun it after `mcu update` or installing a new toolchain (`. host/env.sh; host/patch-rpath.sh <sdk-dir>`).
Works without any env, so `./mcu libcheck` and tools run outside env.sh also see the libs. `minichlink` is rebuilt against static libusb (libudev from `lib/` for hidapi).

## `./mcu libcheck`
Lists host ELF binaries whose libraries resolve outside mcu/ (glibc excluded; libstdc++/libgcc_s reported as gcc runtime from the OS).
`libcheck.py` has a short `OPTIONAL_UNUSED` allowlist: esp-idf `*-gdb-3.{9,10,11,14}` (IDF venv is 3.12), `cmake-gui`, wch `examples_usb` test programs.

## Still taken from the OS
Linux kernel, glibc (+ libstdc++/libgcc_s of the OS gcc), the base utilities in `sysbin.list` (symlinked in `sysbin/`), `/etc/ssl` CA certificates
(downloads in `mcu update`), udev rules (root installs `<sdk>/udev/*.rules`), USB/serial device permissions.

## Updating (`./mcu update host`)
Dry run lists installed -> latest for buildtools-extended (newest 6.0.x on downloads.yoctoproject.org, `.sha256sum` checked), cmake, ninja, uv, patchelf,
the managed CPython patch releases (3.8/3.12/3.13; `uv python install --no-bin`, uv repoints the `cpython-X.Y-linux-x86_64-gnu` minor links, the previous link target is recorded for rollback)
and the private libs. Everything is installed side by side (`cmake-4.4.4/`, `uv-0.13.5/`, `ninja-1.13.2/`, `patchelf-0.18.0/`, `buildtools-6.0.4/`) behind stable symlinks (`cmake`, `uv`, `ninja`, `patchelf`, `buildtools`; `bin/uv`, `bin/ninja`, `bin/patchelf` link into them), so env.sh is never edited and keeps working mid-update.
The private libs (zstd zlib ncurses libusb eudev) are report-only: a rebuild changes the ABI behind every RUNPATH'd vendor binary, so it is done by hand (flags above), then `host/patch-rpath.sh`.
Old versioned dirs are kept; `./mcu prune host` moves the ones older than the previous version to the Trash on request.
