/* (1) TIMER1 interrupts at 200 Hz give a binary semaphore; a handler task takes it and counts the events.
 * (2) A counting semaphore (3 tokens) guards a pool of 3 "DMA channels"; 5 workers compete for them and the
 *     test checks that never more than 3 are in use at once and that all 3 were in use at some point. */
#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"
#include "board.h"

static SemaphoreHandle_t irq_sem, pool;
static volatile uint32_t isr_count, handled, in_use, max_in_use, jobs_done;

void board_timer1_isr(void)
{
  BaseType_t woken = pdFALSE;
  isr_count++;
  xSemaphoreGiveFromISR(irq_sem, &woken);
  portYIELD_FROM_ISR(woken);
}

static void handler(void *arg)
{
  for (;;)
    if (xSemaphoreTake(irq_sem, portMAX_DELAY) == pdTRUE) handled++;
}

static void worker(void *arg)
{
  for (int job = 0; job < 6; job++) {
    xSemaphoreTake(pool, portMAX_DELAY);
    taskENTER_CRITICAL();
    if (++in_use > max_in_use) max_in_use = in_use;
    taskEXIT_CRITICAL();
    vTaskDelay(pdMS_TO_TICKS(3 + (uint32_t)arg));       /* hold the channel for a while */
    taskENTER_CRITICAL();
    in_use--; jobs_done++;
    taskEXIT_CRITICAL();
    xSemaphoreGive(pool);
  }
  vTaskDelete(NULL);
}

static void checker(void *arg)
{
  board_timer1_start(200);
  vTaskDelay(pdMS_TO_TICKS(300));
  board_timer1_stop();
  vTaskDelay(pdMS_TO_TICKS(20));
  log_printf("timer ISR %lu, handled %lu; jobs %lu, max channels in use %lu\n", (unsigned long)isr_count,
             (unsigned long)handled, (unsigned long)jobs_done, (unsigned long)max_in_use);
  test_check(isr_count >= 40, "timer interrupt ran (>= 40 in 300 ms)");
  test_check(handled > 0 && handled <= isr_count, "handler took the binary semaphore (never more than given)");
  test_check(jobs_done == 5 * 6, "all 30 jobs finished");
  test_check(max_in_use == 3, "pool limit held: at most 3 at once, and all 3 used");
  test_check(uxSemaphoreGetCount(pool) == 3, "all tokens returned");
  test_done();
}

int main(void)
{
  log_printf("qemu-semaphores: ISR -> binary semaphore, counting semaphore pool\n");
  irq_sem = xSemaphoreCreateBinary();
  pool = xSemaphoreCreateCounting(3, 3);
  xTaskCreate(handler, "handler", 256, NULL, 3, NULL);
  for (uint32_t i = 0; i < 5; i++) xTaskCreate(worker, "worker", 256, (void *)i, 2, NULL);
  xTaskCreate(checker, "checker", 512, NULL, 1, NULL);
  vTaskStartScheduler();
  return 1;
}
