/* @NAME@: everything blocks between jobs, so the kernel stops the tick and sleeps (WFI) until the next job. */
#include "FreeRTOS.h"
#include "task.h"
#include "board.h"

volatile unsigned long app_sleeps;
extern volatile uint32_t board_systick_count;

static void job(void *arg)
{
  TickType_t last = xTaskGetTickCount();
  for (int i = 0; i < 5; i++) {
    xTaskDelayUntil(&last, pdMS_TO_TICKS(50));   /* the periodic work goes here */
  }
  log_printf("ticks %lu, tick interrupts %lu, sleeps %lu\n", (unsigned long)xTaskGetTickCount(),
             (unsigned long)board_systick_count, app_sleeps);
  test_check(board_systick_count < xTaskGetTickCount() / 2, "tick suppressed while idle");
  test_done();
}

int main(void)
{
  xTaskCreate(job, "job", 512, NULL, 1, NULL);
  vTaskStartScheduler();
  return 1;
}
