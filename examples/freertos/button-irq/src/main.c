/* button-irq: the board button raises an EXTI interrupt; the ISR (board_button_hook, priority 6 >= the syscall ceiling 5) only
 * sends a direct-to-task notification. The button task wakes, debounces (30 ms re-check), toggles the LED and logs the count.
 * Button: BlackPill KEY PA0 (active low, pull-up); WeAct G431 and mini H743 user key PC13 (active high, pull-down). */
#include "board.h"
#include "FreeRTOS.h"
#include "task.h"
#include "rtos_glue.h"

static TaskHandle_t btn_h;
static volatile uint32_t irqs;

void board_button_hook(void)               /* runs in the EXTI ISR */
{
  BaseType_t woken = pdFALSE;
  irqs++;
  if (btn_h) vTaskNotifyGiveFromISR(btn_h, &woken);
  portYIELD_FROM_ISR(woken);
}

static void button_task(void *arg)
{
  (void)arg;
  uint32_t presses = 0;
  for (;;) {
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);   /* one wake-up per burst of edges */
    vTaskDelay(pdMS_TO_TICKS(30));             /* debounce: still pressed after 30 ms? */
    if (!board_button_pressed()) continue;
    board_led_toggle();
    rtos_log("press #%lu (%lu edge interrupts so far)", (unsigned long)++presses, (unsigned long)irqs);
    while (board_button_pressed()) vTaskDelay(pdMS_TO_TICKS(10));   /* wait for release */
    ulTaskNotifyTake(pdTRUE, 0);                                     /* drop bounce edges seen meanwhile */
  }
}

int main(void)
{
  board_init();
  rtos_log_init();
  rtos_print("\r\nfreertos button-irq on %s: press the user button\r\n", BOARD_NAME);
  xTaskCreate(button_task, "button", 256, NULL, 2, &btn_h);
  board_button_init(1);                        /* EXTI on the press edge, NVIC priority 6 */
  vTaskStartScheduler();
  for (;;) {}
}
