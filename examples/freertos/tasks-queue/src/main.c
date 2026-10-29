/* Producer (100 ms) -> queue(8) -> slow consumer (300 ms). The queue fills, so the producer reports dropped messages. */
#include "board.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "rtos_glue.h"

typedef struct { uint32_t seq, tick; } msg_t;
static QueueHandle_t q;

static void producer(void *arg)
{
  (void)arg;
  uint32_t seq = 0, dropped = 0;
  TickType_t last = xTaskGetTickCount();
  for (;;) {
    msg_t m = { seq++, xTaskGetTickCount() };
    if (xQueueSend(q, &m, 0) != pdPASS && (++dropped % 10) == 1) rtos_log("producer: queue full, %lu dropped so far", (unsigned long)dropped);
    vTaskDelayUntil(&last, pdMS_TO_TICKS(100));
  }
}
static void consumer(void *arg)
{
  (void)arg;
  msg_t m;
  for (;;) {
    if (xQueueReceive(q, &m, portMAX_DELAY) == pdPASS) {
      rtos_log("consumer: seq=%lu sent@%lu waiting=%u", (unsigned long)m.seq, (unsigned long)m.tick, (unsigned)uxQueueMessagesWaiting(q));
      board_led_toggle();
      vTaskDelay(pdMS_TO_TICKS(300));
    }
  }
}

int main(void)
{
  board_init();
  rtos_log_init();
  q = xQueueCreate(8, sizeof(msg_t));
  xTaskCreate(producer, "producer", 256, NULL, 2, NULL);
  xTaskCreate(consumer, "consumer", 384, NULL, 1, NULL);
  vTaskStartScheduler();
  for (;;) {}
}
