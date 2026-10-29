/* Three tasks connected by two queues: producer -> filter (keeps even numbers, scales by 3) -> consumer.
 * The consumer checks order and checksum; queue sizes are small on purpose, so the producer blocks on a full queue. */
#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"
#include "board.h"

#define N_ITEMS 200
typedef struct { uint32_t seq; uint32_t value; } item_t;

static QueueHandle_t q_raw, q_filtered;
static volatile uint32_t producer_blocked;

static void producer(void *arg)
{
  for (uint32_t i = 1; i <= N_ITEMS; i++) {
    item_t it = { i, i };
    if (xQueueSend(q_raw, &it, 0) != pdPASS) {   /* full: count it, then wait for space */
      producer_blocked++;
      xQueueSend(q_raw, &it, portMAX_DELAY);
    }
  }
  item_t end = { 0, 0 };
  xQueueSend(q_raw, &end, portMAX_DELAY);
  vTaskDelete(NULL);
}

static void filter(void *arg)
{
  item_t it;
  for (;;) {
    xQueueReceive(q_raw, &it, portMAX_DELAY);
    if (it.seq == 0 || (it.value % 2) == 0) {
      it.value *= 3;
      xQueueSend(q_filtered, &it, portMAX_DELAY);
    }
    if (it.seq == 0) vTaskDelete(NULL);
  }
}

static void consumer(void *arg)
{
  item_t it;
  uint32_t count = 0, sum = 0, last = 0, in_order = 1;
  for (;;) {
    if (xQueueReceive(q_filtered, &it, pdMS_TO_TICKS(2000)) != pdPASS) { test_check(0, "consumer timed out"); break; }
    if (it.seq == 0) break;
    if (it.seq <= last) in_order = 0;
    last = it.seq; count++; sum += it.value;
  }
  uint32_t want = 0;
  for (uint32_t i = 2; i <= N_ITEMS; i += 2) want += 3 * i;
  log_printf("received %lu items, sum %lu (want %lu), producer blocked %lu times\n",
             (unsigned long)count, (unsigned long)sum, (unsigned long)want, (unsigned long)producer_blocked);
  test_check(count == N_ITEMS / 2, "every even item arrived");
  test_check(sum == want, "checksum matches");
  test_check(in_order, "items arrived in order");
  test_check(producer_blocked > 0, "producer blocked on a full queue at least once");
  test_check(uxQueueMessagesWaiting(q_raw) == 0, "raw queue drained");
  test_done();
}

int main(void)
{
  log_printf("qemu-queue-pipeline: producer -> filter -> consumer\n");
  q_raw = xQueueCreate(4, sizeof(item_t));
  q_filtered = xQueueCreate(4, sizeof(item_t));
  vQueueAddToRegistry(q_raw, "raw");
  vQueueAddToRegistry(q_filtered, "filtered");
  xTaskCreate(producer, "producer", 256, NULL, 2, NULL);
  xTaskCreate(filter, "filter", 256, NULL, 2, NULL);
  xTaskCreate(consumer, "consumer", 512, NULL, 1, NULL);
  vTaskStartScheduler();
  return 1;
}
