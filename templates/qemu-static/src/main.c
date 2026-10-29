/* @NAME@: static allocation only (RAM use fixed at link time; check with the size output or the .map). */
#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"
#include "board.h"

#define WORKER_STACK 256
static StaticTask_t worker_tcb, main_tcb;
static StackType_t worker_stack[WORKER_STACK], main_stack[512];
static StaticQueue_t q_tcb;
static uint8_t q_storage[4 * sizeof(uint32_t)];
static QueueHandle_t q;

static void worker(void *arg)
{
  for (uint32_t i = 0; i < 5; i++) xQueueSend(q, &i, portMAX_DELAY);
  vTaskSuspend(NULL);
}

static void main_task(void *arg)
{
  uint32_t v, n = 0;
  while (n < 5 && xQueueReceive(q, &v, pdMS_TO_TICKS(100)) == pdPASS) n++;
  test_check(n == 5, "five items over a static queue");
  test_done();
}

int main(void)
{
  q = xQueueCreateStatic(4, sizeof(uint32_t), q_storage, &q_tcb);
  xTaskCreateStatic(worker, "worker", WORKER_STACK, NULL, 2, worker_stack, &worker_tcb);
  xTaskCreateStatic(main_task, "main", 512, NULL, 1, main_stack, &main_tcb);
  vTaskStartScheduler();
  return 1;
}
