/* Software timers run their callbacks in the timer service task. This example uses an auto-reload timer
 * (10 ms), a one-shot timer that is reset before it expires (so it fires later, once), the timer ID as a
 * per-timer counter, xTimerChangePeriod() and xTimerPendFunctionCall(). */
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "board.h"

static volatile uint32_t oneshot_fired, pended_value;
static volatile TickType_t oneshot_at;

static void periodic_cb(TimerHandle_t t)
{
  uint32_t n = (uint32_t)pvTimerGetTimerID(t);     /* the ID doubles as a counter */
  vTimerSetTimerID(t, (void *)(n + 1));
}

static void oneshot_cb(TimerHandle_t t)
{
  oneshot_fired++;
  oneshot_at = xTaskGetTickCount();
}

static void pended(void *p1, uint32_t p2) { pended_value = (uint32_t)p1 + p2; }

static void checker(void *arg)
{
  TimerHandle_t periodic = xTimerCreate("periodic", pdMS_TO_TICKS(10), pdTRUE, (void *)0, periodic_cb);
  TimerHandle_t oneshot = xTimerCreate("oneshot", pdMS_TO_TICKS(50), pdFALSE, NULL, oneshot_cb);
  TickType_t t0 = xTaskGetTickCount();
  xTimerStart(periodic, 0);
  xTimerStart(oneshot, 0);
  vTaskDelay(pdMS_TO_TICKS(30));
  xTimerReset(oneshot, 0);                          /* restart: now expires 50 ms after this point */
  vTaskDelay(pdMS_TO_TICKS(175));                   /* t = 205 ms */
  uint32_t n = (uint32_t)pvTimerGetTimerID(periodic);
  log_printf("periodic fired %lu times in ~205 ms; one-shot fired %lu time(s) at +%lu ms\n", (unsigned long)n,
             (unsigned long)oneshot_fired, (unsigned long)(oneshot_at - t0));
  test_check(n >= 19 && n <= 21, "auto-reload 10 ms timer fired about 20 times");
  test_check(oneshot_fired == 1, "one-shot fired exactly once");
  test_check(oneshot_at - t0 >= pdMS_TO_TICKS(78) && oneshot_at - t0 <= pdMS_TO_TICKS(85), "reset moved the one-shot to ~80 ms");
  test_check(xTimerIsTimerActive(oneshot) == pdFALSE, "one-shot is dormant after firing");

  vTimerSetTimerID(periodic, (void *)0);
  xTimerChangePeriod(periodic, pdMS_TO_TICKS(25), 0);
  vTaskDelay(pdMS_TO_TICKS(105));
  n = (uint32_t)pvTimerGetTimerID(periodic);
  test_check(n >= 3 && n <= 5, "changed period (25 ms) gives about 4 callbacks in 105 ms");
  xTimerStop(periodic, 0);

  xTimerPendFunctionCall(pended, (void *)40, 2, 0);
  vTaskDelay(pdMS_TO_TICKS(5));
  test_check(pended_value == 42, "pended function ran in the timer task");
  test_done();
}

int main(void)
{
  log_printf("qemu-sw-timers: auto-reload, one-shot, ID, period change, pend call\n");
  xTaskCreate(checker, "checker", 512, NULL, 1, NULL);
  vTaskStartScheduler();
  return 1;
}
