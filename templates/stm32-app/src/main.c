/* @NAME@: LED task + log task on the board console (USART1 PA9/PA10, 115200). */
#include "board.h"
#include "FreeRTOS.h"
#include "task.h"
#include "rtos_glue.h"

static void blink(void *arg)
{
  for (;;) { board_led_toggle(); vTaskDelay(pdMS_TO_TICKS(500)); }
}

static void logger(void *arg)
{
  for (uint32_t n = 0;; n++) {
    rtos_log("@NAME@ alive %lu", (unsigned long)n);
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
