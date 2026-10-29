# source: . toolchain/freertos/env.sh   (bash or zsh; scripts/app does it for you)
if [ -n "${BASH_SOURCE:-}" ]; then _s="${BASH_SOURCE[0]}"; else _s="${(%):-%x}"; fi
_F="$(cd -P "$(dirname "$_s")/.." && pwd -P)"; unset _s   # -P: reached via a symlink (e.g. mcu's tool/freertos) it must still be this ID's toolchain/
. "$_F/host/env.sh"
. "$_F/stm32/env.sh"
export FREERTOS_KERNEL="$_F/freertos/FreeRTOS-Kernel"
export FREERTOS_QEMU="$_F/freertos/qemu"                 # qemu mps2-an385 board layer (examples qemu-*)
export PATH="$_F/qemu/qemu-arm/bin:$PATH"                # xPack qemu-system-arm (scripts/setup)
unset _F
