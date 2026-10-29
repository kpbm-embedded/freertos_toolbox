#!/usr/bin/env bash
# Exit 3 (skip) when 32_31_bend is missing: Bend is reached there, never downloaded here.
here="$(cd -P "$(dirname "$0")" && pwd -P)"
x="${BOX_BEND:-$here/../../../../32_31_bend}/bend-box"
[ -x "$x" ] || { echo "SKIP: needs 32_31_bend (bend-box) next to this ID or BOX_BEND"; exit 3; }
