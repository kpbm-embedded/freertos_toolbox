/* @NAME@: two tasks and a queue on qemu mps2-an385. Replace the work in sender()/receiver() with yours;
 * keep the test_check()/test_done() calls (or remove them for an endless app). */
#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"
#include "board.h"

static QueueHandle_t q;

static void sender(void *arg)
{
  for (uint32_t i = 1; i <= 10; i++) {
    xQueueSend(q, &i, portMAX_DELAY);
    vTaskDelay(pdMS_TO_TICKS(10));
  }
  vTaskDelete(NULL);
}

static void receiver(void *arg)
{
  uint32_t v, sum = 0;
  for (int i = 0; i < 10; i++) {
    xQueueReceive(q, &v, portMAX_DELAY);
    log_printf("[%lu ms] got %lu\n", (unsigned long)xTaskGetTickCount(), (unsigned long)v);
    sum += v;
  }
  test_check(sum == 55, "received 1..10");
  test_done();
}

int main(void)
{
  log_printf("@NAME@ starting\n");
  q = xQueueCreate(4, sizeof(uint32_t));
  xTaskCreate(sender, "sender", 256, NULL, 2, NULL);
  xTaskCreate(receiver, "receiver", 512, NULL, 1, NULL);
  vTaskStartScheduler();
  return 1;
}
