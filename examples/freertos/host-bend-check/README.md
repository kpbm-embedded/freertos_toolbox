# host-bend-check

Bend 2 where it fits FreeRTOS work: on the host, as a reference for task logic. A FreeRTOS task on qemu sums the
samples used by qemu-rust-task; `src/stats.bend` computes the same with Bend on the host; `test.sh` compares them.
Bend has no MCU/RTOS target (its C output needs a hosted POSIX system), so nothing Bend runs on the chip.

Needs `32_31_bend` (`bend-box`, `$BOX_BEND`); without it `needs.sh` exits 3 and `scripts/app examples test` reports
SKIP. Written before that ID existed: the Bend file has not been checked by `bend` yet.
