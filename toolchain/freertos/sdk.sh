SDK_DESC="FreeRTOS-Kernel + STM32F411 examples (own copy of the stm32 Cube pack)"
SDK_SELFCHECK="blackpill-f411 blinky-f411-freertos"
FREERTOS_TC="$(cd -P "$SDK_DIR/.." && pwd -P)"   # -P: mcu reaches this dir via a symlink; .. must be this ID's toolchain/   # 32_04_freertos/toolchain: self-contained copies of host, tools, stm32
source "$FREERTOS_TC/stm32/sdk-common.sh"
EX_DIR="$FREERTOS_TC/freertos/examples"; DEFAULT_EX=blinky-f411-freertos   # physical: the examples climb ../../../stm32, which must be this ID's
sdk_env() { source "$SDK_DIR/env.sh"; }
sdk_doctor() {
  common_doctor; local rc=$?
  [[ -f "$SDK_DIR/FreeRTOS-Kernel/tasks.c" ]] && echo "ok      FreeRTOS-Kernel $({ git -C "$SDK_DIR/FreeRTOS-Kernel" describe --tags 2>/dev/null || git -C "$SDK_DIR/FreeRTOS-Kernel" branch --show-current; })" || { echo "MISSING FreeRTOS-Kernel"; rc=1; }
  [[ -d "$FREERTOS_TC/stm32/STM32CubeF4/Drivers" ]] && echo "ok      STM32CubeF4 (from toolchain/stm32)" || { echo "MISSING STM32CubeF4"; rc=1; }
  return $rc
}
sdk_versions() { echo "FreeRTOS-Kernel $({ git -C "$SDK_DIR/FreeRTOS-Kernel" describe --tags 2>/dev/null || git -C "$SDK_DIR/FreeRTOS-Kernel" branch --show-current; })"; echo "STM32CubeF4 $(git -C "$FREERTOS_TC/stm32/STM32CubeF4" describe --tags 2>/dev/null)"; }
sdk_targets() { common_targets; }
sdk_examples() { common_examples; }
sdk_build() { common_build "$@"; }
sdk_flash() { common_flash "$@"; }
sdk_monitor() { common_monitor "$@"; }
sdk_clean() { common_clean "$@"; }
_f_do() { local new="$1"; upd_git_rec FreeRTOS-Kernel
  git -C "$SDK_DIR/FreeRTOS-Kernel" fetch -q --depth 1 origin "refs/tags/$new:refs/tags/$new" && git -C "$SDK_DIR/FreeRTOS-Kernel" checkout -q "$new" && git -C "$SDK_DIR/FreeRTOS-Kernel" submodule update --init --recursive --depth 1; }
sdk_update() {
  UPD_RC=0
  local cur new="${1:-}"; cur=$({ git -C "$SDK_DIR/FreeRTOS-Kernel" describe --tags 2>/dev/null || git -C "$SDK_DIR/FreeRTOS-Kernel" branch --show-current; })
  [[ -n "$new" && "$new" != all ]] || new=$(upd_git_newest_tag https://github.com/FreeRTOS/FreeRTOS-Kernel '^V[0-9]+\.[0-9]+\.[0-9]+$')
  upd_step FreeRTOS-Kernel "$cur" "$new" _f_do "$new"
  upd_dry_hint
  return "$UPD_RC"
}
