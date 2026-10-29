/* Trace hooks: called by the kernel; keep them tiny. */
void app_trace_switch(void);
#define traceTASK_SWITCHED_IN()            app_trace_switch()
unsigned long board_cycles(void);
#define configGENERATE_RUN_TIME_STATS            1
#define portCONFIGURE_TIMER_FOR_RUN_TIME_STATS() do { } while (0)
#define portGET_RUN_TIME_COUNTER_VALUE()         (board_cycles() / 25u)   /* microseconds */
