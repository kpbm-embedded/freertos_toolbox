/* Trace macros: the kernel calls them at each event; keep them short (they run inside the kernel, often with
 * interrupts masked). pxCurrentTCB is the kernel's current-task pointer, visible where the macros expand. */
void trace_switched_in(void *tcb);
void trace_queue_op(int is_send);
#define traceTASK_SWITCHED_IN()        trace_switched_in((void *)pxCurrentTCB)
#define traceQUEUE_SEND(q)             trace_queue_op(1)
#define traceQUEUE_RECEIVE(q)          trace_queue_op(0)

/* Run-time stats: a 25 MHz free-running counter (TIMER0, started by board_init) divided by 25 = microseconds */
unsigned long board_cycles(void);
#define configGENERATE_RUN_TIME_STATS            1
#define portCONFIGURE_TIMER_FOR_RUN_TIME_STATS() do { } while (0)
#define portGET_RUN_TIME_COUNTER_VALUE()         (board_cycles() / 25u)
