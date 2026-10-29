#!/usr/bin/env bash
# Compare the firmware's sum (qemu.log) with the Bend reference run on the host (32_31_bend).
# Called as test.sh <app.elf> (scripts/app) or test.sh <target> <outdir> (32_01_mcu check.sh).
set -euo pipefail
here="$(cd -P "$(dirname "$0")" && pwd -P)"
bb="${BOX_BEND:-$here/../../../../32_31_bend}/bend-box"
[ -x "$bb" ] || { echo "SKIP: needs 32_31_bend"; exit 3; }
dir="$(dirname "$1")"; [ -f "$1" ] || dir="$2"
want="$(grep -rho 'sum = -\?[0-9]*' "$dir" --include='*.log' | head -1 | awk '{print $NF}')"
got="$("$bb" run bend "$here/src/stats.bend" | tail -1 | tr -dc '0-9-')"
echo "test.sh: firmware $want, Bend $got"
[ "$want" = "$got" ]
