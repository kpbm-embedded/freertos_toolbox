/* A tiny trace recorder: the last 64 context switches (task handle + timestamp) and queue operation counters. */
#include <stdint.h>
#include "trace.h"

volatile uint32_t trace_switches, trace_sends, trace_receives;
trace_event_t trace_ring[TRACE_RING];
void *volatile trace_watch[2];
volatile uint32_t trace_watch_hits[2];

void trace_switched_in(void *tcb)
{
  extern uint32_t board_cycles(void);
  trace_event_t *e = &trace_ring[trace_switches % TRACE_RING];
  e->task = tcb;
  e->cycles = board_cycles();
  trace_switches++;
  if (tcb == trace_watch[0]) trace_watch_hits[0]++;
  if (tcb == trace_watch[1]) trace_watch_hits[1]++;
}

void trace_queue_op(int is_send)
{
  if (is_send) trace_sends++; else trace_receives++;
}
