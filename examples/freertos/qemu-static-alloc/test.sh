#!/usr/bin/env bash
# Extra check after the build/run: no FreeRTOS heap in the image.
# Called as test.sh <app.elf> (scripts/app examples test) or test.sh <target> <outdir> (32_01_mcu check.sh).
set -euo pipefail
elf="$1"
[[ -f $elf ]] || elf="$(find "${2:?}" -maxdepth 2 -name 'qemu-static-alloc.elf' | head -1)"
[[ -f $elf ]] || { echo "test.sh: no ELF found"; exit 1; }
if arm-none-eabi-nm "$elf" | grep -qE ' (pvPortMalloc|vPortFree|ucHeap)$'; then
  echo "test.sh: heap symbols found in $elf"; exit 1
fi
echo "test.sh: no pvPortMalloc/vPortFree/ucHeap in $(basename "$elf")"
