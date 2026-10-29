#!/usr/bin/env bash
# Exit 3 (skip) when 32_24_microzig is missing: the Zig compiler is reached there, not copied.
here="$(cd -P "$(dirname "$0")" && pwd -P)"
x="${BOX_MICROZIG:-$here/../../../../32_24_microzig}/microzig-box"
[ -x "$x" ] || { echo "SKIP: needs 32_24_microzig (microzig-box) next to this ID or BOX_MICROZIG"; exit 3; }
