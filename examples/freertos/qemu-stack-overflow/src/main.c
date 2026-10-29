/* Method 2 stack checking: the kernel fills each stack with 0xA5 and checks the last 16 bytes at every context
 * switch. "greedy" (a 1 KB stack) calls a function with a 1.2 KB local array; at its next switch the hook runs
 * and names the task. First, high-water marks of a well-sized task are printed and checked. */
#include <string.h>
#include "FreeRTOS.h"
#include "task.h"
#include "board.h"

static volatile int phase;

void vApplicationStackOverflowHook(TaskHandle_t task, char *name)
{
  log_printf("stack overflow hook: task '%s'\n", name);
  test_check(phase == 1, "the overflow came from greedy (the expected one)");
  test_check(strcmp(name, "greedy") == 0, "the hook named the overflowing task");
  test_done();
}

static __attribute__((noinline)) uint32_t use_stack(uint32_t n)
{
  volatile uint8_t big[1200];
  memset((void *)big, (int)n, sizeof big);
  return big[n % sizeof big];
}

static void greedy(void *arg)
{
  phase = 1;
  use_stack(7);
  vTaskDelay(1);                          /* context switch: the check runs here */
  test_check(0, "overflow was not detected");
  test_done();
}

static void checker(void *arg)
{
  UBaseType_t hw = uxTaskGetStackHighWaterMark(NULL);
  log_printf("checker stack high-water mark: %u words free of 512\n", (unsigned)hw);
  test_check(hw > 100 && hw < 512, "high-water mark of a healthy task is plausible");
  xTaskCreate(greedy, "greedy", 256, NULL, 2, NULL);   /* 256 words = 1 KB */
  vTaskDelay(pdMS_TO_TICKS(100));
  test_check(0, "hook never ran");
  test_done();
}

int main(void)
{
  log_printf("qemu-stack-overflow: method 2 detection\n");
  xTaskCreate(checker, "checker", 512, NULL, 1, NULL);
  vTaskStartScheduler();
  return 1;
}
