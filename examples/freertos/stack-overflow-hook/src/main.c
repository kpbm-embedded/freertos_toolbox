/* configCHECK_FOR_STACK_OVERFLOW = 2: the kernel paints each stack and checks the last bytes at every context switch.
 * "victim" recurses until it overruns its 320-word stack; vApplicationStackOverflowHook() reports the task, flashes the LED, then resets. */
#include <string.h>
#include "board.h"
#include "FreeRTOS.h"
#include "task.h"
#include "rtos_glue.h"

static void put(const char *s) { HAL_UART_Transmit(&board_uart, (uint8_t *)s, (uint16_t)strlen(s), 100); }

void vApplicationStackOverflowHook(TaskHandle_t t, char *name)
{
  (void)t;
  __asm volatile("cpsid i");                      /* the kernel is no longer trustworthy: no RTOS calls from here on */
  put("\r\n*** STACK OVERFLOW in task '"); put(name); put("' - resetting ***\r\n");
  for (int i = 0; i < 20; i++) {
    board_led_toggle();
    for (volatile uint32_t d = 0; d < BOARD_SYSCLK_HZ / 40U; d++) {}
  }
  NVIC_SystemReset();
}

static volatile uint32_t sink;
static void recurse(unsigned depth)
{
  volatile uint8_t pad[96];
  memset((void *)pad, (int)depth, sizeof pad);
  sink += pad[depth % sizeof pad];
  rtos_log("victim: depth %u, stack high-water %u words", depth, (unsigned)uxTaskGetStackHighWaterMark(NULL));
  vTaskDelay(pdMS_TO_TICKS(100));                 /* context switch -> overflow check */
  recurse(depth + 1);
  sink += pad[0];                                 /* defeat tail-call optimisation */
}
static void victim(void *a) { (void)a; vTaskDelay(pdMS_TO_TICKS(1000)); recurse(0); }
static void healthy(void *a) { (void)a; for (;;) { vTaskDelay(pdMS_TO_TICKS(500)); board_led_toggle(); rtos_log("healthy task alive"); } }

int main(void)
{
  board_init();
  rtos_log_init();
  rtos_print("\r\nstack-overflow-hook demo on %s\r\n", BOARD_NAME);
  xTaskCreate(healthy, "healthy", 256, NULL, 1, NULL);
  xTaskCreate(victim, "victim", 320, NULL, 1, NULL);
  vTaskStartScheduler();
  for (;;) {}
}
