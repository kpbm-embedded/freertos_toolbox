/* @NAME@: LED task + log task on the board console (USART1 PA9/PA10, 115200). */
#include "board.h"
#include "FreeRTOS.h"
#include "task.h"
#include "rtos_glue.h"
#include "filter.h"

static void blink(void *arg)
{
  for (;;) { board_led_toggle(); vTaskDelay(pdMS_TO_TICKS(500)); }
}

static void logger(void *arg)
{
  for (uint32_t n = 0;; n++) {
    static filter_t f; filter_add(&f, (int32_t)(n % 7) * 100);
    rtos_log("@NAME@ n=%lu avg=%ld", (unsigned long)n, (long)filter_avg(&f));
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

int main(void)
{
  board_init();
  rtos_log_init();
  xTaskCreate(blink, "blink", 128, NULL, 1, NULL);
  xTaskCreate(logger, "log", 384, NULL, 1, NULL);
  vTaskStartScheduler();
  for (;;) { }
}
