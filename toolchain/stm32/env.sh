# shellcheck shell=bash
# stm32 environment: source it from bash or zsh (. toolchain/stm32/env.sh).
# Gives the host layer, the Arm toolchain and flash tools (tools/env.sh) and the STM32Cube pack paths.
# In bash BASH_SOURCE names this file; in a sourced zsh file $0 does.
if [ -n "${BASH_SOURCE:-}" ]; then _mcu_env_file="${BASH_SOURCE[0]}"; else _mcu_env_file="$0"; fi
_mcu_root="$(cd -P "$(dirname "$_mcu_env_file")/.." && pwd -P)"   # -P: stay in this ID even when reached via a symlink
. "$_mcu_root/host/env.sh"
. "$_mcu_root/tools/env.sh"

# Component locations (one place; later resolved from pinned versions instead of fixed dirs)
export STM32_CUBE_F4="$_mcu_root/stm32/STM32CubeF4"
export STM32_CUBE_F1="$_mcu_root/stm32/STM32CubeF1"
export STM32_CUBE_G4="$_mcu_root/stm32/STM32CubeG4"
export STM32_CUBE_H7="$_mcu_root/stm32/STM32CubeH7"
unset _mcu_env_file _mcu_root
