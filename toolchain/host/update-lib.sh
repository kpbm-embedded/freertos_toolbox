# shellcheck shell=bash
# Compatibility shim: the update helpers moved to ../lib/update.sh and the plugin helpers to ../lib/helpers.sh.
# Anything that still sources host/update-lib.sh gets both. New code: rely on ../mcu loading them.
_mcu_lib_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")/../lib" && pwd)"
# shellcheck source=lib/update.sh
source "$_mcu_lib_dir/update.sh"
# shellcheck source=lib/helpers.sh
source "$_mcu_lib_dir/helpers.sh"
unset _mcu_lib_dir
