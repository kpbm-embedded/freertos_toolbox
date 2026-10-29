/* @NAME@: trace hooks + run-time stats. Print the stats table whenever you need it (here: after 200 ms). */
#include "FreeRTOS.h"
#include "task.h"
#include "board.h"

static volatile uint32_t switches;
void app_trace_switch(void) { switches++; }

static void worker(void *arg) { for (;;) { for (volatile int i = 0; i < 5000; i++) { } vTaskDelay(1); } }

static void monitor(void *arg)
{
  static char table[512];
  vTaskDelay(pdMS_TO_TICKS(200));
  vTaskGetRunTimeStatistics(table, sizeof table);
  log_printf("context switches: %lu\n%s", (unsigned long)switches, table);
  test_check(switches > 100, "trace hook counted the switches");
  test_done();
}

int main(void)
{
  xTaskCreate(worker, "worker", 256, NULL, 1, NULL);
  xTaskCreate(monitor, "monitor", 1024, NULL, 2, NULL);
  vTaskStartScheduler();
  return 1;
}
