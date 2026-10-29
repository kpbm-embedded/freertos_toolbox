/* SysTick belongs to FreeRTOS, so the HAL timebase is rebuilt on the DWT cycle counter before the scheduler starts and on
 * the RTOS tick afterwards. Also: default malloc/stack-overflow hooks and a mutex-protected console. */
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include "board.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "rtos_glue.h"

HAL_StatusTypeDef HAL_InitTick(uint32_t prio)
{
  (void)prio;
  CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
#if defined(BOARD_H7)
  DWT->LAR = 0xC5ACCE55U;                      /* Cortex-M7 DWT is lock-protected */
#endif
  DWT->CYCCNT = 0;
  DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
  return HAL_OK;
}

uint32_t HAL_GetTick(void)
{
  if (xTaskGetSchedulerState() == taskSCHEDULER_NOT_STARTED) return DWT->CYCCNT / (SystemCoreClock / 1000U);
  return xTaskGetTickCount();
}

__attribute__((weak)) void vApplicationMallocFailedHook(void)
{
  __asm volatile("cpsid i");
  for (;;) {}
}
__attribute__((weak)) void vApplicationStackOverflowHook(TaskHandle_t t, char *name)
{
  (void)t; (void)name;
  __asm volatile("cpsid i");
  for (;;) {}
}

static SemaphoreHandle_t log_mx;
void rtos_log_init(void) { log_mx = xSemaphoreCreateMutex(); }

static void emit(const char *b, int n)
{
  int run = xTaskGetSchedulerState() == taskSCHEDULER_RUNNING && log_mx;
  if (run) xSemaphoreTake(log_mx, portMAX_DELAY);
  HAL_UART_Transmit(&board_uart, (uint8_t *)b, (uint16_t)n, 1000);
  if (run) xSemaphoreGive(log_mx);
}
void rtos_print(const char *fmt, ...)
{
  char b[160];
  va_list ap; va_start(ap, fmt);
  int n = vsnprintf(b, sizeof b, fmt, ap);
  va_end(ap);
  if (n > (int)sizeof b - 1) n = sizeof b - 1;
  emit(b, n);
}
void rtos_log(const char *fmt, ...)
{
  char b[160];
  int n = snprintf(b, sizeof b, "[%7lu] ", (unsigned long)xTaskGetTickCount());
  va_list ap; va_start(ap, fmt);
  n += vsnprintf(b + n, sizeof b - 3 - (size_t)n, fmt, ap);
  va_end(ap);
  if (n > (int)sizeof b - 3) n = sizeof b - 3;
  b[n++] = '\r'; b[n++] = '\n';
  emit(b, n);
}
