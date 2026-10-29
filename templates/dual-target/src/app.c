/* @NAME@: hardware-independent app code (only FreeRTOS + platform.h). */
#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"
#include "platform.h"

static QueueHandle_t q;

static void blinker(void *arg)
{
  for (int i = 0; i < 6; i++) { platform_led(i & 1); xQueueSend(q, &i, portMAX_DELAY); vTaskDelay(pdMS_TO_TICKS(50)); }
  vTaskSuspend(NULL);
}

static void reporter(void *arg)
{
  int v, n = 0;
  while (n < 6 && xQueueReceive(q, &v, pdMS_TO_TICKS(500)) == pdPASS) { platform_log("led step %d", v); n++; }
  platform_finish(n == 6);
  for (;;) vTaskDelay(portMAX_DELAY);
}

int main(void)
{
  platform_init();
  q = xQueueCreate(4, sizeof(int));
  xTaskCreate(blinker, "blinker", 256, NULL, 2, NULL);
  xTaskCreate(reporter, "reporter", 512, NULL, 1, NULL);
  vTaskStartScheduler();
  for (;;) { }
}
