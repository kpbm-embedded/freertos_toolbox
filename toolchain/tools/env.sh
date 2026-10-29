# source this: . toolchain/tools/env.sh   (bash or zsh)
if [ -n "${BASH_SOURCE:-}" ]; then _t="${BASH_SOURCE[0]}"; else _t="${(%):-%x}"; fi
_T="$(cd -P "$(dirname "$_t")" && pwd -P)"; unset _t
. "$_T/../host/env.sh"
export PATH="$_T/toolchains/arm-gnu-toolchain/bin:$_T/venv/bin:$_T/bin:$_T/xpack-openocd/bin:$_T/stlink/bin:$PATH"
export ARM_GNU_TOOLCHAIN="$_T/toolchains/arm-gnu-toolchain"
export OPENOCD_SCRIPTS="$_T/xpack-openocd/openocd/scripts"
export STLINK_CHIPS_DIR="$_T/stlink/share/chips"
export UV_PYTHON_INSTALL_DIR="$_T/python"   # tools/python holds CPython 3.13 used by tools/venv; always 'uv python install --no-bin'
unset _T
