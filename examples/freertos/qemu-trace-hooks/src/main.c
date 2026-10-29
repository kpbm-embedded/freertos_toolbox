/* Trace hooks feed a ring buffer of context switches and queue counters (what Tracealyzer/SystemView recorders
 * do at a larger scale). Run-time stats (vTaskGetRunTimeStatistics) show how CPU time was shared: "busy"
 * spins, "ping"/"pong" exchange 100 messages over a queue. */
#include <string.h>
#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"
#include "board.h"
#include "trace.h"

static QueueHandle_t q;
static TaskHandle_t ping_h, pong_h, busy_h;

static void ping(void *arg) { for (uint32_t i = 0; i < 100; i++) { xQueueSend(q, &i, portMAX_DELAY); vTaskDelay(1); } vTaskSuspend(NULL); }
static void pong(void *arg) { uint32_t v; for (;;) xQueueReceive(q, &v, portMAX_DELAY); }
static void busy(void *arg) { for (;;) { for (volatile int i = 0; i < 2000; i++) { } taskYIELD(); } }

static void report(void *arg)
{
  static char stats[512];
  vTaskDelay(pdMS_TO_TICKS(250));
  vTaskSuspend(busy_h);
  vTaskGetRunTimeStatistics(stats, sizeof stats);
  log_printf("run-time stats (task, us, %%):\n%s", stats);
  log_printf("last context switches (task @ cycle):\n");
  for (uint32_t i = trace_switches - 6; i != trace_switches; i++) {
    trace_event_t *e = &trace_ring[i % TRACE_RING];
    log_printf("  %-8s @ %lu\n", pcTaskGetName((TaskHandle_t)e->task), (unsigned long)e->cycles);
  }
  log_printf("switches %lu, queue sends %lu, receives %lu\n", (unsigned long)trace_switches,
             (unsigned long)trace_sends, (unsigned long)trace_receives);
  test_check(trace_switches > 200, "traceTASK_SWITCHED_IN recorded the context switches");
  test_check(trace_sends == 100 && trace_receives == 100, "traceQUEUE_SEND/RECEIVE counted every message");
  log_printf("switch-ins: ping %lu, pong %lu\n", (unsigned long)trace_watch_hits[0], (unsigned long)trace_watch_hits[1]);
  test_check(trace_watch_hits[0] >= 100 && trace_watch_hits[1] >= 100, "switch-ins into ping and pong seen by the hook");
  test_check(strstr(stats, "busy") && strstr(stats, "ping") && strstr(stats, "IDLE"), "run-time stats list the tasks");
  TaskStatus_t st;
  vTaskGetInfo(busy_h, &st, pdFALSE, eInvalid);
  configRUN_TIME_COUNTER_TYPE busy_time = st.ulRunTimeCounter;
  vTaskGetInfo(ping_h, &st, pdFALSE, eInvalid);
  test_check(busy_time > st.ulRunTimeCounter, "the busy task used more CPU than ping");
  test_done();
}

int main(void)
{
  log_printf("qemu-trace-hooks: trace macros and run-time stats\n");
  q = xQueueCreate(4, sizeof(uint32_t));
  xTaskCreate(ping, "ping", 256, NULL, 2, &ping_h);
  xTaskCreate(pong, "pong", 256, NULL, 3, &pong_h);
  xTaskCreate(busy, "busy", 256, NULL, 1, &busy_h);
  xTaskCreate(report, "report", 1024, NULL, 4, NULL);
  trace_watch[0] = ping_h;
  trace_watch[1] = pong_h;
  vTaskStartScheduler();
  return 1;
}
