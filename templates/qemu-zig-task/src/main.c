/* C side: a producer task sends 100 samples, the Zig task (src/task.zig) computes statistics and sends them
 * back; this file checks them. The Zig code calls the kernel only through rtos_shim.c. */
#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"
#include "board.h"

typedef struct { uint32_t count; int32_t min, max, sum; } stats_t;
void zig_stats_task(void *params);
static void *queues[2];

static void producer(void *arg)
{
  for (int32_t i = 0; i < 100; i++) {
    int32_t s = (i * 37) % 101 - 50;          /* -50..50 */
    xQueueSend(queues[0], &s, portMAX_DELAY);
  }
  int32_t end = INT32_MIN;
  xQueueSend(queues[0], &end, portMAX_DELAY);
  stats_t st;
  if (xQueueReceive(queues[1], &st, pdMS_TO_TICKS(1000)) != pdPASS) { test_check(0, "no result from the Zig task"); test_done(); }
  int32_t want_sum = 0, mn = INT32_MAX, mx = INT32_MIN;
  for (int32_t i = 0; i < 100; i++) { int32_t s = (i * 37) % 101 - 50; want_sum += s; if (s < mn) mn = s; if (s > mx) mx = s; }
  log_printf("Zig stats: count %lu min %ld max %ld sum %ld\n", (unsigned long)st.count, (long)st.min, (long)st.max, (long)st.sum);
  test_check(st.count == 100, "Zig task received every sample");
  test_check(st.min == mn && st.max == mx && st.sum == want_sum, "Zig task statistics match the C reference");
  test_done();
}

int main(void)
{
  log_printf("@NAME@: a FreeRTOS task written in Zig\n");
  queues[0] = xQueueCreate(8, sizeof(int32_t));
  queues[1] = xQueueCreate(1, sizeof(stats_t));
  xTaskCreate(zig_stats_task, "zig", 512, queues, 2, NULL);
  xTaskCreate(producer, "producer", 512, NULL, 1, NULL);
  vTaskStartScheduler();
  return 1;
}
