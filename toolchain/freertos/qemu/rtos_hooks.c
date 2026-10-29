/* FreeRTOS hooks shared by the qemu examples. Each one reports and ends the run with RESULT: FAIL, unless the
 * example defines its own (they are weak). */
#include "FreeRTOS.h"
#include "task.h"
#include "board.h"

__attribute__((weak)) void board_assert_failed(const char *file, int line)
{
  log_printf("ASSERT failed: %s:%d\nRESULT: FAIL (assert)\n", file, line);
  board_exit(1);
}

__attribute__((weak)) void vApplicationStackOverflowHook(TaskHandle_t task, char *name)
{
  (void)task;
  log_printf("stack overflow in task %s\nRESULT: FAIL (stack overflow)\n", name);
  board_exit(1);
}

#if configUSE_MALLOC_FAILED_HOOK
__attribute__((weak)) void vApplicationMallocFailedHook(void)
{
  log_printf("pvPortMalloc failed\nRESULT: FAIL (malloc)\n");
  board_exit(1);
}
#endif
