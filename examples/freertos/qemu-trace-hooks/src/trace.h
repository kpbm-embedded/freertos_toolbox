#pragma once
#include <stdint.h>
#define TRACE_RING 64
typedef struct { void *task; uint32_t cycles; } trace_event_t;
extern volatile uint32_t trace_switches, trace_sends, trace_receives;
extern trace_event_t trace_ring[TRACE_RING];
extern void *volatile trace_watch[2];            /* two task handles whose switch-ins are counted */
extern volatile uint32_t trace_watch_hits[2];
