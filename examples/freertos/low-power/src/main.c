/* low-power: tickless idle. With configUSE_TICKLESS_IDLE=1 (src/FreeRTOSConfig.h) the idle task stops the 1 kHz SysTick whenever
 * every task is blocked for >= 5 ticks, programs SysTick for the whole idle period and sleeps in WFI; the tick count is corrected on
 * wake-up. The only work here is a 50 ms LED flash every 2 s and a report every 10 s, so the core sleeps most of the time.
 * The pre/post-sleep hooks count the sleeps and the idle ticks they were planned for. This is Sleep mode (core clock gated),
 * not Stop/Standby: SysTick needs the core clock, so the console and all clocks keep working. */
#include "board.h"
#include "FreeRTOS.h"
#include "task.h"
#include "rtos_glue.h"

static volatile uint32_t sleeps, planned_ticks;

void lp_pre_sleep(uint32_t expected_ticks) { sleeps++; planned_ticks += expected_ticks; }
void lp_post_sleep(uint32_t expected_ticks) { (void)expected_ticks; }

static void flash_task(void *arg)
{
  (void)arg;
  TickType_t last = xTaskGetTickCount();
  for (;;) {
    board_led_set(1);
    vTaskDelay(pdMS_TO_TICKS(50));
    board_led_set(0);
    vTaskDelayUntil(&last, pdMS_TO_TICKS(2000));
  }
}

static void report_task(void *arg)
{
  (void)arg;
  uint32_t s0 = 0, p0 = 0;
  for (;;) {
    vTaskDelay(pdMS_TO_TICKS(10000));
    uint32_t s = sleeps, p = planned_ticks;
    rtos_log("last 10 s: %lu tickless sleeps planned for %lu of 10000 ticks (~%lu %%; an interrupt can end one early)",
             (unsigned long)(s - s0), (unsigned long)(p - p0), (unsigned long)((p - p0) / 100U));
    s0 = s; p0 = p;
  }
}

int main(void)
{
  board_init();
  rtos_log_init();
  rtos_print("\r\nfreertos low-power (tickless idle) on %s\r\n", BOARD_NAME);
  xTaskCreate(flash_task, "flash", configMINIMAL_STACK_SIZE, NULL, 2, NULL);
  xTaskCreate(report_task, "report", 256, NULL, 1, NULL);
  vTaskStartScheduler();
  for (;;) {}
}
