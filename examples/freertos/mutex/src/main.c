/* Priority inheritance: LOW (prio 1) holds the mutex for 1 s, HIGH (prio 3) wants it. While HIGH waits, the kernel boosts LOW to
 * priority 3 (printed), so MID (prio 2, CPU-hungry) cannot starve it. */
#include "board.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "rtos_glue.h"

static SemaphoreHandle_t mx;
static TaskHandle_t h_low;

static void low(void *a)
{
  (void)a;
  for (;;) {
    xSemaphoreTake(mx, portMAX_DELAY);
    rtos_log("LOW : took mutex, prio=%u", (unsigned)uxTaskPriorityGet(NULL));
    vTaskDelay(pdMS_TO_TICKS(300));
    rtos_log("LOW : still holding, prio=%u (boosted if HIGH is waiting)", (unsigned)uxTaskPriorityGet(NULL));
    TickType_t t0 = xTaskGetTickCount();
    while (xTaskGetTickCount() - t0 < pdMS_TO_TICKS(200)) {}   /* work while holding */
    rtos_log("LOW : releasing, prio=%u", (unsigned)uxTaskPriorityGet(NULL));
    xSemaphoreGive(mx);
    vTaskDelay(pdMS_TO_TICKS(2000));
  }
}
static void mid(void *a)
{
  (void)a;
  for (;;) {                                                    /* CPU hog bursts */
    TickType_t t0 = xTaskGetTickCount();
    while (xTaskGetTickCount() - t0 < pdMS_TO_TICKS(150)) {}
    vTaskDelay(pdMS_TO_TICKS(100));
  }
}
static void high(void *a)
{
  (void)a;
  vTaskDelay(pdMS_TO_TICKS(100));                               /* let LOW grab the mutex first */
  for (;;) {
    TickType_t t0 = xTaskGetTickCount();
    if (xSemaphoreTake(mx, pdMS_TO_TICKS(3000)) == pdPASS) {
      rtos_log("HIGH: got mutex after %lu ms", (unsigned long)(xTaskGetTickCount() - t0));
      board_led_toggle();
      xSemaphoreGive(mx);
    } else rtos_log("HIGH: timeout");
    vTaskDelay(pdMS_TO_TICKS(2000));
  }
}

int main(void)
{
  board_init();
  rtos_log_init();
  mx = xSemaphoreCreateMutex();                                 /* mutexes (not binary semaphores) do priority inheritance */
  xTaskCreate(low, "low", 256, NULL, 1, &h_low);
  xTaskCreate(mid, "mid", 256, NULL, 2, NULL);
  xTaskCreate(high, "high", 256, NULL, 3, NULL);
  vTaskStartScheduler();
  for (;;) {}
}
