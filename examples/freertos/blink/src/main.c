/* blink: one FreeRTOS task toggles the board LED every 500 ms with vTaskDelayUntil (drift-free period); a second, lower-priority
 * task reports the LED task's stack high-water mark once every 5 s on USART1 (PA9 TX, 115200 8N1). */
#include "board.h"
#include "FreeRTOS.h"
#include "task.h"
#include "rtos_glue.h"

static TaskHandle_t led_h;
static volatile uint32_t toggles;

static void led_task(void *arg)
{
  (void)arg;
  TickType_t last = xTaskGetTickCount();
  for (;;) {
    board_led_toggle();
    toggles++;
    vTaskDelayUntil(&last, pdMS_TO_TICKS(500));
  }
}

static void report_task(void *arg)
{
  (void)arg;
  for (;;) {
    vTaskDelay(pdMS_TO_TICKS(5000));
    rtos_log("blink: %lu toggles, led stack free %u words", (unsigned long)toggles, (unsigned)uxTaskGetStackHighWaterMark(led_h));
  }
}

int main(void)
{
  board_init();
  rtos_log_init();
  rtos_print("\r\nfreertos blink on %s\r\n", BOARD_NAME);
  xTaskCreate(led_task, "led", configMINIMAL_STACK_SIZE, NULL, 2, &led_h);
  xTaskCreate(report_task, "report", 256, NULL, 1, NULL);
  vTaskStartScheduler();
  for (;;) {}
}
