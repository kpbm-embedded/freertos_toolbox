# qemu-heap-stats

heap_4 statistics: fragmentation and coalescing seen through vPortGetHeapStats, minimum-ever free, a failed allocation.

Runs on qemu `mps2-an385` (Cortex-M3, FreeRTOS ARM_CM3 port, board layer `toolchain/freertos/qemu/`); no board needed.
Features: freertos, heap_4, memory, qemu. The program checks itself and ends with `RESULT: PASS` (qemu exit 0) or `RESULT: FAIL` (exit 1).

## Run

```sh
scripts/app run qemu-heap-stats              # build + run in qemu
scripts/app examples test qemu-heap-stats    # the same, as a test (also runs test.sh if present)
```

## Output (qemu, 2026-10-29)

```
qemu-heap-stats: heap_4 fragmentation and coalescing
start          free 10640 in 1 block(s), largest 10640, min-ever 10640, allocs 7 frees 0
A B C          free  6616 in 1 block(s), largest  6616, min-ever  6616, allocs 10 frees 0
CHECK ok  : three blocks allocated
free B         free  8624 in 2 block(s), largest  6616, min-ever  6616, allocs 10 frees 1
CHECK ok  : freeing B leaves a hole (one more free block)
CHECK ok  : largest block < total free: fragmented
free A         free  9632 in 2 block(s), largest  6616, min-ever  6616, allocs 10 frees 2
CHECK ok  : A merged with the hole of B
free C         free 10640 in 1 block(s), largest 10640, min-ever  6616, allocs 10 frees 3
CHECK ok  : all merged back
CHECK ok  : all memory returned
CHECK ok  : minimum-ever free remembers the peak
CHECK ok  : counters: 3 allocations, 3 frees
CHECK ok  : too-large request failed and called the hook once
RESULT: PASS (9 checks)
```

On a board: the code uses only the kernel API plus `board.h` (UART log, a timer interrupt, a software interrupt); port the three board calls to run it on an STM32.
