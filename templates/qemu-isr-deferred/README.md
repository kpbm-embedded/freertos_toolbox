# @NAME@

ISR -> task handoff on qemu: task notification + stream buffer.

Pattern: the ISR only copies data and signals; the task does the work. On a board, replace the TIMER1 ISR with the peripheral ISR and set its NVIC priority to a value >= `configMAX_SYSCALL_INTERRUPT_PRIORITY`.

## Build and run

```sh
scripts/app run apps/@NAME@
```

Made from the freertos-box template `qemu-isr-deferred` (`scripts/app new qemu-isr-deferred @NAME@`).
