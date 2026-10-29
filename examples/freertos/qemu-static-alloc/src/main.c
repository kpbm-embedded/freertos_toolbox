/* Every kernel object is created from memory the application owns (xTaskCreateStatic, xQueueCreateStatic, ...).
 * Nothing is allocated at run time, so the RAM use is known at link time (see the size output). The idle and
 * timer service tasks get their memory from vApplicationGet{Idle,Timer}TaskMemory below. */
#include "FreeRTOS.h"
#include "event_groups.h"
#include "queue.h"
#include "semphr.h"
#include "stream_buffer.h"
#include "task.h"
#include "timers.h"
#include "board.h"

static StaticTask_t idle_tcb, timer_tcb, prod_tcb, cons_tcb;
static StackType_t idle_stack[configMINIMAL_STACK_SIZE], timer_stack[configTIMER_TASK_STACK_DEPTH];
static StackType_t prod_stack[256], cons_stack[512];
static StaticQueue_t q_storage; static uint8_t q_buf[8 * sizeof(uint32_t)];
static StaticSemaphore_t mtx_storage;
static StaticEventGroup_t eg_storage;
static StaticTimer_t tmr_storage;
static StaticStreamBuffer_t sb_storage; static uint8_t sb_buf[64 + 1];

static QueueHandle_t q; static SemaphoreHandle_t mtx; static EventGroupHandle_t eg;
static TimerHandle_t tmr; static StreamBufferHandle_t sb;
static volatile uint32_t timer_hits;

void vApplicationGetIdleTaskMemory(StaticTask_t **tcb, StackType_t **stack, configSTACK_DEPTH_TYPE *depth)
{
  *tcb = &idle_tcb; *stack = idle_stack; *depth = configMINIMAL_STACK_SIZE;
}

void vApplicationGetTimerTaskMemory(StaticTask_t **tcb, StackType_t **stack, configSTACK_DEPTH_TYPE *depth)
{
  *tcb = &timer_tcb; *stack = timer_stack; *depth = configTIMER_TASK_STACK_DEPTH;
}

static void tick(TimerHandle_t t) { timer_hits++; xEventGroupSetBits(eg, 0x2); }

static void producer(void *arg)
{
  for (uint32_t i = 1; i <= 10; i++) xQueueSend(q, &i, portMAX_DELAY);
  xStreamBufferSend(sb, "static", 6, portMAX_DELAY);
  xEventGroupSetBits(eg, 0x1);
  vTaskSuspend(NULL);
}

static void consumer(void *arg)
{
  uint32_t v, sum = 0; char text[8] = { 0 };
  for (int i = 0; i < 10; i++) { xQueueReceive(q, &v, portMAX_DELAY); xSemaphoreTake(mtx, portMAX_DELAY); sum += v; xSemaphoreGive(mtx); }
  xStreamBufferReceive(sb, text, 6, pdMS_TO_TICKS(100));
  xTimerStart(tmr, 0);
  EventBits_t b = xEventGroupWaitBits(eg, 0x3, pdFALSE, pdTRUE, pdMS_TO_TICKS(200));
  test_check(sum == 55, "queue: 1..10 summed under the mutex");
  test_check(text[0] == 's' && text[5] == 'c', "stream buffer carried the text");
  test_check((b & 0x3) == 0x3, "event group: producer bit and timer bit");
  test_check(timer_hits >= 1, "static timer fired");
  test_check(eTaskGetState(xTaskGetHandle("producer")) == eSuspended, "static producer task suspended itself");
  test_done();
}

int main(void)
{
  log_printf("qemu-static-alloc: no heap, all objects static\n");
  q = xQueueCreateStatic(8, sizeof(uint32_t), q_buf, &q_storage);
  mtx = xSemaphoreCreateMutexStatic(&mtx_storage);
  eg = xEventGroupCreateStatic(&eg_storage);
  tmr = xTimerCreateStatic("tick", pdMS_TO_TICKS(10), pdFALSE, NULL, tick, &tmr_storage);
  sb = xStreamBufferCreateStatic(sizeof sb_buf - 1, 1, sb_buf, &sb_storage);
  xTaskCreateStatic(producer, "producer", 256, NULL, 2, prod_stack, &prod_tcb);
  xTaskCreateStatic(consumer, "consumer", 512, NULL, 1, cons_stack, &cons_tcb);
  vTaskStartScheduler();
  return 1;
}
