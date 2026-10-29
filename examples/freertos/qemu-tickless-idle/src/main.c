/* With tickless idle the kernel stops the periodic tick while every task is blocked and sleeps (WFI) until the
 * next task is due, then corrects the tick count. A task wakes every 100 ms ten times. Expected: the tick count
 * advances by ~1000, but far fewer SysTick interrupts are taken (board_systick_count), and the pre/post-sleep
 * hooks ran. The 25 MHz TIMER0 cycle counter checks that kernel time matches wall time. */
#include "FreeRTOS.h"
#include "task.h"
#include "board.h"

volatile unsigned long app_sleeps, app_wakeups;
extern volatile uint32_t board_systick_count;

static void sleeper(void *arg)
{
  vTaskDelay(pdMS_TO_TICKS(10));
  TickType_t t0 = xTaskGetTickCount();
  uint32_t irq0 = board_systick_count, c0 = board_cycles();
  for (int i = 0; i < 10; i++) vTaskDelay(pdMS_TO_TICKS(100));
  uint32_t ticks = xTaskGetTickCount() - t0, irqs = board_systick_count - irq0;
  uint32_t ms = (board_cycles() - c0) / (BOARD_CPU_HZ / 1000);
  log_printf("ticks %lu, SysTick interrupts %lu, cycle-counter time %lu ms, sleeps %lu, wakeups %lu\n",
             (unsigned long)ticks, (unsigned long)irqs, (unsigned long)ms, app_sleeps, app_wakeups);
  test_check(ticks >= 990 && ticks <= 1010, "tick count advanced by ~1000 (10 x 100 ms)");
  test_check(irqs < ticks / 4, "far fewer SysTick interrupts than ticks: the tick was suppressed");
  test_check(app_sleeps >= 10 && app_wakeups == app_sleeps, "pre/post sleep hooks ran in pairs");
  test_check(ms >= 900 && ms <= 1300, "kernel time agrees with the hardware counter");
  test_done();
}

int main(void)
{
  log_printf("qemu-tickless-idle: tick suppression while idle\n");
  xTaskCreate(sleeper, "sleeper", 512, NULL, 1, NULL);
  vTaskStartScheduler();
  return 1;
}
