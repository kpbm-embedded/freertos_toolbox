/* @NAME@: an active object. The door task owns the state machine (lib/fsm.c) and its event queue; other code
 * only posts events. Actions are carried out here (motor, timer); the logic is tested on the host. */
#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"
#include "timers.h"
#include "board.h"
#include "fsm.h"

static QueueHandle_t events;
static TimerHandle_t open_timer;
static volatile int motor_stops;

static void post(fsm_event_t ev) { xQueueSend(events, &ev, 0); }
static void on_timeout(TimerHandle_t t) { post(EV_TIMEOUT); }

static void door(void *arg)
{
  fsm_state_t s = fsm_initial();
  fsm_event_t ev;
  for (;;) {
    xQueueReceive(events, &ev, portMAX_DELAY);
    fsm_action_t a = fsm_step(&s, ev);
    log_printf("[%4lu ms] event %d -> %s\n", (unsigned long)xTaskGetTickCount(), (int)ev, fsm_state_name(s));
    switch (a) {
    case ACT_MOTOR_OPEN: case ACT_MOTOR_CLOSE: case ACT_NONE: break;
    case ACT_MOTOR_STOP_START_TIMER: motor_stops++; xTimerStart(open_timer, 0); break;
    case ACT_MOTOR_STOP: motor_stops++;
      test_check(s == ST_CLOSED && motor_stops == 2, "door went round: open, wait, close");
      test_done();
    }
  }
}

static void simulator(void *arg)          /* stands in for the button and limit switches */
{
  post(EV_BUTTON);
  vTaskDelay(pdMS_TO_TICKS(30)); post(EV_LIMIT_OPEN);
  vTaskDelay(pdMS_TO_TICKS(80));          /* the 50 ms timer fires in between: EV_TIMEOUT */
  post(EV_LIMIT_CLOSED);
  vTaskSuspend(NULL);
}

int main(void)
{
  events = xQueueCreate(8, sizeof(fsm_event_t));
  open_timer = xTimerCreate("open", pdMS_TO_TICKS(50), pdFALSE, NULL, on_timeout);
  xTaskCreate(door, "door", 512, NULL, 2, NULL);
  xTaskCreate(simulator, "sim", 256, NULL, 1, NULL);
  vTaskStartScheduler();
  return 1;
}
