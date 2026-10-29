#!/usr/bin/env bash
# Exit 3 (skip) when 32_17_embedded_rust is missing: the Rust compiler is reached there, not copied.
here="$(cd -P "$(dirname "$0")" && pwd -P)"
x="${BOX_EMBEDDED_RUST:-${FREERTOS_KERNEL:-$here/x/x}/../../../../32_17_embedded_rust}/rust-box"
[ -x "$x" ] || { echo "SKIP: needs 32_17_embedded_rust (rust-box) next to this ID or BOX_EMBEDDED_RUST"; exit 3; }
