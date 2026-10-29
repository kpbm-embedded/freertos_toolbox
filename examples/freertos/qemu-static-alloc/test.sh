#!/usr/bin/env bash
# Extra check after the qemu run: no FreeRTOS heap in the image (scripts/app examples test passes the ELF).
set -euo pipefail
elf="$1"
if arm-none-eabi-nm "$elf" | grep -qE ' (pvPortMalloc|vPortFree|ucHeap)$'; then
  echo "test.sh: heap symbols found in $elf"; exit 1
fi
echo "test.sh: no pvPortMalloc/vPortFree/ucHeap in the ELF"
