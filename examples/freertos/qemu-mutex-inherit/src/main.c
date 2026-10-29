/* Priority inversion and its fix. LOW takes the mutex; HIGH then blocks on it; MEDIUM is ready and would starve
 * LOW. With a mutex (not a binary semaphore) the kernel lends LOW the priority of HIGH until it gives the mutex
 * back, so MEDIUM cannot preempt it. The test reads LOW's priority while it holds the mutex. Then a recursive
 * mutex is taken three times and must be given three times. */
#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"
#include "board.h"

enum { P_LOW = 1, P_MED = 2, P_HIGH = 3 };
static SemaphoreHandle_t mtx, rec;
static TaskHandle_t low_h;
static volatile UBaseType_t low_prio_while_holding;
static volatile int medium_ran_during_hold, holding, high_done;

static void low(void *arg)
{
  xSemaphoreTake(mtx, portMAX_DELAY);
  vTaskDelay(pdMS_TO_TICKS(5));                 /* HIGH and MEDIUM wake up and HIGH blocks on the mutex */
  holding = 1;                                  /* from here LOW runs with HIGH's priority */
  for (volatile int i = 0; i < 20000; i++) { }  /* work while holding: MEDIUM must not get the CPU now */
  low_prio_while_holding = uxTaskPriorityGet(NULL);
  holding = 0;
  xSemaphoreGive(mtx);
  vTaskSuspend(NULL);
}

static void medium(void *arg)
{
  vTaskDelay(pdMS_TO_TICKS(2));
  for (;;) { if (holding) medium_ran_during_hold = 1; if (high_done) vTaskSuspend(NULL); taskYIELD(); }
}

static void high(void *arg)
{
  vTaskDelay(pdMS_TO_TICKS(2));
  xSemaphoreTake(mtx, portMAX_DELAY);
  xSemaphoreGive(mtx);
  high_done = 1;
  test_check(low_prio_while_holding == P_HIGH, "LOW inherited HIGH's priority while holding the mutex");
  test_check(uxTaskPriorityGet(low_h) == P_LOW, "LOW is back at its own priority after the give");
  test_check(!medium_ran_during_hold, "MEDIUM did not run while LOW held the mutex (no inversion)");
  int depth = 0;
  for (int i = 0; i < 3; i++) depth += xSemaphoreTakeRecursive(rec, 0) == pdTRUE;
  xSemaphoreGiveRecursive(rec); xSemaphoreGiveRecursive(rec);
  test_check(depth == 3 && xSemaphoreGetMutexHolder(rec) == xTaskGetCurrentTaskHandle(), "recursive mutex: still held after 2 of 3 gives");
  xSemaphoreGiveRecursive(rec);
  test_check(xSemaphoreGetMutexHolder(rec) == NULL, "recursive mutex: free after the 3rd give");
  test_done();
}

int main(void)
{
  log_printf("qemu-mutex-inherit: priority inheritance and recursive mutex\n");
  mtx = xSemaphoreCreateMutex();
  rec = xSemaphoreCreateRecursiveMutex();
  xTaskCreate(low, "low", 256, NULL, P_LOW, &low_h);
  xTaskCreate(medium, "medium", 256, NULL, P_MED, NULL);
  xTaskCreate(high, "high", 512, NULL, P_HIGH, NULL);
  vTaskStartScheduler();
  return 1;
}
