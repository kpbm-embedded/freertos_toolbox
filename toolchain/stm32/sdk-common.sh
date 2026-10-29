# Shared helpers for stm32/ and freertos/ plugins. Needs: MCU_ROOT SDK_DIR MCU_OUT MCU_JOBS; EX_DIR (examples dir); DEFAULT_EX.
STM32_HELPERS="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd -P)"   # this copy (32_04_freertos/toolchain/stm32), self-contained
_tgt_ok() { case "${1:-}" in blackpill-f411) ;; *) echo "unknown target '${1:-}' (see: mcu targets $SDK)" >&2; return 1;; esac; }
_chk() { local n="$1"; shift; local v; if v=$("$@" 2>&1 | head -1) && command -v "$1" >/dev/null; then echo "ok      $n  $v"; else echo "MISSING $n"; return 1; fi; }
common_doctor() {
  local rc=0
  _chk arm-none-eabi-gcc arm-none-eabi-gcc --version; local _gcc _tc; _gcc="$(command -v arm-none-eabi-gcc || true)"; _tc="$(cd "$STM32_HELPERS/../tools/toolchains" 2>/dev/null && pwd -P)"   # this plugin's own toolchain tree
  [[ -n "$_gcc" && -n "$_tc" && "$(readlink -f "$_gcc")" == "$_tc"/* ]] || echo "warn    arm-none-eabi-gcc is not the bundled toolchain (${_gcc:-none})"
  _chk cmake cmake --version || rc=1
  _chk ninja ninja --version || rc=1
  _chk st-flash st-flash --version || rc=1
  _chk openocd openocd --version || rc=1
  _chk pyocd pyocd --version || rc=1
  _chk probe-rs probe-rs --version || rc=1
  command -v dfu-util >/dev/null && echo "ok      dfu-util" || echo "warn    dfu-util not installed (optional; DFU flashing unavailable)"
  return $rc
}
common_examples() { local d; for d in "$EX_DIR"/*/; do [[ -f "$d/CMakeLists.txt" ]] && basename "$d"; done; }
common_targets() { echo "blackpill-f411  WeAct BlackPill STM32F411CEU6 (owned)"; }
common_build() {
  local t="${1:-}" ex="${2:-$DEFAULT_EX}"; _tgt_ok "$t" || return 1
  [[ "$ex" == -- ]] && ex="$DEFAULT_EX"
  local extra=(); local i; for i in "$@"; do [[ "$i" == -- ]] && { shift $(( $(printf '%s\n' "$@" | grep -n -x -- -- | head -1 | cut -d: -f1) )); extra=("$@"); break; }; done
  [[ -d "$EX_DIR/$ex" ]] || { echo "unknown example '$ex' (have: $(common_examples | tr '\n' ' '))" >&2; return 1; }
  local b="$MCU_OUT/$t/$ex"
  cmake -S "$EX_DIR/$ex" -B "$b" -G Ninja "${extra[@]}" >/dev/null
  cmake --build "$b" -j"$MCU_JOBS" 2>&1 | grep -E 'error|FLASH:|RAM:|^ +[0-9]+\s' || true
  [[ -f "$b/$ex.bin" ]] || { echo "build failed: $b" >&2; return 1; }
  ls -1 "$b/$ex".elf "$b/$ex".bin "$b/$ex".hex
}
common_flash() {
  local t="${1:-}" ex="${2:-$DEFAULT_EX}" probe="${3:-st-flash}"; _tgt_ok "$t" || return 1
  local b="$MCU_OUT/$t/$ex"; [[ -f "$b/$ex.bin" ]] || { echo "not built: mcu build $SDK $t $ex" >&2; return 1; }
  case "$probe" in
    st-flash) st-flash --reset write "$b/$ex.bin" 0x08000000 ;;
    openocd)  openocd -f interface/stlink.cfg -f target/stm32f4x.cfg -c "program $b/$ex.elf verify reset exit" ;;
    pyocd)    pyocd flash -t stm32f411ceux "$b/$ex.elf" ;;
    probe-rs) probe-rs download --chip STM32F411CEUx "$b/$ex.elf" && probe-rs reset --chip STM32F411CEUx ;;
    dfu)      command -v dfu-util >/dev/null || { echo "dfu-util not installed; hold BOOT0, reset, then use it" >&2; return 1; }
              dfu-util -a 0 -d 0483:df11 -s 0x08000000:leave -D "$b/$ex.bin" ;;
    *) echo "probe must be st-flash|openocd|pyocd|probe-rs|dfu" >&2; return 1 ;;
  esac
}
common_monitor() { python -m serial.tools.miniterm "${1:-/dev/ttyUSB0}" "${2:-115200}"; }
common_clean() {
  local t="${1:-}" ex="${2:-$DEFAULT_EX}"; _tgt_ok "$t" || return 1
  local b="$MCU_OUT/$t/$ex"; [[ -d "$b" ]] || { echo "nothing to clean"; return 0; }
  mkdir -p ~/.local/share/Trash/files; mv "$b" ~/.local/share/Trash/files/"mcu-$SDK-$t-$ex-$(date +%s)"; echo "moved $b to Trash"
}
common_new() {
  local name="${1:-}" t="${2:-blackpill-f411}"; [[ -n "$name" ]] || { echo "usage: mcu new $SDK <name> [target]" >&2; return 1; }
  _tgt_ok "$t" || return 1
  local dst="$STM32_HELPERS/../../apps/$name"; [[ -e "$dst" ]] && { echo "$dst exists" >&2; return 1; }
  mkdir -p "$dst"; cp -r "$STM32_HELPERS/examples/blinky-f411/src" "$dst/src"
  cp "$STM32_HELPERS/examples/common/"{arm-none-eabi.cmake,stm32f411.cmake,stm32f4xx_hal_conf.h,STM32F411CEUx_FLASH.ld,clock_f411.h} "$dst/"
  sed -i 's|CMAKE_CURRENT_LIST_DIR}/../../STM32CubeF4|STM32_HELPERS}/STM32CubeF4|; s|set(CUBE .*|set(CUBE '"$STM32_HELPERS"'/STM32CubeF4 CACHE PATH "STM32CubeF4")|' "$dst/stm32f411.cmake"
  cat > "$dst/CMakeLists.txt" <<EOT
cmake_minimum_required(VERSION 3.20)
set(CMAKE_TOOLCHAIN_FILE \${CMAKE_CURRENT_LIST_DIR}/arm-none-eabi.cmake)
project($name C ASM)
include(\${CMAKE_CURRENT_LIST_DIR}/stm32f411.cmake)
include_directories(\${CMAKE_CURRENT_LIST_DIR})
add_executable($name src/main.c)
target_link_libraries($name stm32f411_hal)
stm32f411_finish($name)
EOT
  echo "created $dst (cmake -S $dst -B build)"
}
