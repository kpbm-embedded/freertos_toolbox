/* @NAME@: the supervisor feeds the (hardware) watchdog only when every worker checked in during the period.
 * Here worker 2 stops checking in after a while; the supervisor must notice it within one period. */
#include "FreeRTOS.h"
#include "event_groups.h"
#include "task.h"
#include "board.h"

#define N_WORKERS 3
#define ALL_ALIVE ((1u << N_WORKERS) - 1)
#define PERIOD_MS 50
static EventGroupHandle_t alive;

static void worker(void *arg)
{
  uint32_t me = (uint32_t)arg;
  for (int round = 0;; round++) {
    if (me == 2 && round == 20) vTaskSuspend(NULL);      /* simulated hang */
    xEventGroupSetBits(alive, 1u << me);
    vTaskDelay(pdMS_TO_TICKS(10 + me * 5));
  }
}

static void supervisor(void *arg)
{
  int healthy_periods = 0;
  for (;;) {
    vTaskDelay(pdMS_TO_TICKS(PERIOD_MS));
    EventBits_t b = xEventGroupClearBits(alive, ALL_ALIVE);   /* returns the bits before clearing */
    if ((b & ALL_ALIVE) == ALL_ALIVE) { healthy_periods++; /* feed the watchdog here */ continue; }
    log_printf("supervisor: missing check-in mask 0x%lx after %d healthy periods\n",
               (unsigned long)(~b & ALL_ALIVE), healthy_periods);
    test_check((~b & ALL_ALIVE) == (1u << 2), "the hung worker (2) was identified");
    test_check(healthy_periods >= 3, "healthy periods before the hang");
    test_done();                                              /* on a board: let the watchdog reset */
  }
}

int main(void)
{
  alive = xEventGroupCreate();
  for (uint32_t i = 0; i < N_WORKERS; i++) xTaskCreate(worker, "worker", 256, (void *)i, 1, NULL);
  xTaskCreate(supervisor, "supervisor", 512, NULL, 3, NULL);
  vTaskStartScheduler();
  return 1;
}
