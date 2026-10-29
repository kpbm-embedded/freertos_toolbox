/* Software timers (run in the timer daemon task): 250 ms auto-reload LED toggle, 2 s heartbeat that arms a 500 ms one-shot every 3rd beat. */
#include "board.h"
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "rtos_glue.h"

static TimerHandle_t t_blink, t_beat, t_once;

static void blink_cb(TimerHandle_t t) { (void)t; board_led_toggle(); }
static void once_cb(TimerHandle_t t) { (void)t; rtos_log("one-shot fired"); }
static void beat_cb(TimerHandle_t t)
{
  static unsigned n;
  (void)t;
  rtos_log("heartbeat #%u", n);
  if (++n % 3 == 0) { rtos_log("arming one-shot (500 ms)"); xTimerStart(t_once, 0); }
}

int main(void)
{
  board_init();
  rtos_log_init();
  t_blink = xTimerCreate("blink", pdMS_TO_TICKS(250), pdTRUE, NULL, blink_cb);
  t_beat  = xTimerCreate("beat", pdMS_TO_TICKS(2000), pdTRUE, NULL, beat_cb);
  t_once  = xTimerCreate("once", pdMS_TO_TICKS(500), pdFALSE, NULL, once_cb);
  xTimerStart(t_blink, 0);
  xTimerStart(t_beat, 0);
  vTaskStartScheduler();                          /* timer commands queued above are processed once the daemon runs */
  for (;;) {}
}
