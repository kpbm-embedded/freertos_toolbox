/* @NAME@: keep the ISR short. The timer ISR (1 kHz here; a UART or ADC ISR on a board) puts the sample into a
 * stream buffer and notifies the handler task, which does the slow work at task level. Set the ISR priority
 * numerically >= configMAX_SYSCALL_INTERRUPT_PRIORITY so it may call FromISR functions (BOARD_IRQ_PRIO_LOW). */
#include "FreeRTOS.h"
#include "stream_buffer.h"
#include "task.h"
#include "board.h"

#define SAMPLES 200
static StreamBufferHandle_t samples;
static TaskHandle_t handler_task;
static volatile uint16_t next_sample;

void board_timer1_isr(void)                         /* the interrupt: read the "device", hand over, leave */
{
  BaseType_t woken = pdFALSE;
  uint16_t s = next_sample++;
  xStreamBufferSendFromISR(samples, &s, sizeof s, &woken);
  if ((s % 10) == 9) vTaskNotifyGiveFromISR(handler_task, &woken);   /* wake the task every 10 samples */
  portYIELD_FROM_ISR(woken);
}

static void handler(void *arg)                      /* the deferred part: may block, log, compute */
{
  uint32_t got = 0, sum = 0, wakeups = 0;
  board_timer1_start(1000);
  while (got < SAMPLES) {
    if (ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(100)) == 0) break;
    wakeups++;
    uint16_t s;
    while (xStreamBufferReceive(samples, &s, sizeof s, 0) == sizeof s) { sum += s; got++; }
  }
  board_timer1_stop();
  log_printf("%lu samples in %lu wakeups, sum %lu\n", (unsigned long)got, (unsigned long)wakeups, (unsigned long)sum);
  test_check(got >= SAMPLES, "all samples reached the task");
  test_check(wakeups <= got / 10 + 1, "one wakeup per 10 samples");
  test_done();
}

int main(void)
{
  samples = xStreamBufferCreate(64 * sizeof(uint16_t), sizeof(uint16_t));
  xTaskCreate(handler, "handler", 512, NULL, 3, &handler_task);
  vTaskStartScheduler();
  return 1;
}
