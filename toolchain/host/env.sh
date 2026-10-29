# Shared host layer for every SDK in mcu/: source it first (bash or zsh).
# Gives build tools that do not come from OS packages:
#   buildtools/  Yocto buildtools-extended 6.0.4 (git, make, gcc/g++, tar, patch, wget, m4, pkg-config, python3)
#   cmake/       Kitware CMake 4.4.4 prebuilt        bin/ninja  ninja 1.13.2
#   bin/uv       uv 0.13.5                            python/    uv-managed CPython 3.12 / 3.13 (standalone builds)
# Only glibc, the kernel and the base shell utilities listed in sysbin.list are taken from the OS.
if [ -n "${BASH_SOURCE:-}" ]; then _mh="${BASH_SOURCE[0]}"; else _mh="${(%):-%x}"; fi
MCU_HOST="$(cd "$(dirname "$_mh")" && pwd)"; unset _mh
export MCU_HOST

if [ -z "${MCU_HOST_ACTIVE:-}" ]; then
  # buildtools first, then our tools in front of it (newer cmake/python win over buildtools' ones)
  . "$MCU_HOST/buildtools/environment-setup-x86_64-pokysdk-linux"
  export PATH="$MCU_HOST/bin:$MCU_HOST/cmake/bin:$MCU_HOST/python/cpython-3.12-linux-x86_64-gnu/bin:$PATH"
  export MCU_HOST_ACTIVE=1
fi

# uv: keep interpreters, cache and tool links inside the workspace; never fall back to /usr/bin/python
export UV_PYTHON_INSTALL_DIR="$MCU_HOST/python"
export UV_CACHE_DIR="$MCU_HOST/.uv-cache"
export UV_TOOL_DIR="$MCU_HOST/uv-tools"
export UV_TOOL_BIN_DIR="$MCU_HOST/bin"
export UV_PYTHON_PREFERENCE=only-managed
export UV_PYTHON_INSTALL_BIN=0
export PIP_DISABLE_PIP_VERSION_CHECK=1
