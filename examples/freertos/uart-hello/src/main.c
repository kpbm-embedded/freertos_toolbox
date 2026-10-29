/* uart-hello: a "hello" task prints a greeting once a second on USART1 (PA9 TX / PA10 RX, 115200 8N1). Received bytes arrive in the
 * USART1 RX interrupt (priority 6), go into a stream buffer (xStreamBufferSendFromISR) and an "echo" task sends them back. */
#include "board.h"
#include "FreeRTOS.h"
#include "task.h"
#include "stream_buffer.h"
#include "rtos_glue.h"

static StreamBufferHandle_t rx_sb;
static uint8_t rx_byte;

void USART1_IRQHandler(void) { HAL_UART_IRQHandler(&board_uart); }
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *h)
{
  BaseType_t woken = pdFALSE;
  if (h == &board_uart) {
    xStreamBufferSendFromISR(rx_sb, &rx_byte, 1, &woken);
    HAL_UART_Receive_IT(h, &rx_byte, 1);
  }
  portYIELD_FROM_ISR(woken);
}
void HAL_UART_ErrorCallback(UART_HandleTypeDef *h)
{
  __HAL_UART_CLEAR_OREFLAG(h);
  HAL_UART_Receive_IT(h, &rx_byte, 1);
}

static void hello_task(void *arg)
{
  (void)arg;
  TickType_t last = xTaskGetTickCount();
  for (unsigned n = 0;; n++) {
    rtos_log("hello from FreeRTOS on %s #%u", BOARD_NAME, n);
    board_led_toggle();
    vTaskDelayUntil(&last, pdMS_TO_TICKS(1000));
  }
}

static void echo_task(void *arg)
{
  (void)arg;
  uint8_t buf[32];
  for (;;) {
    size_t n = xStreamBufferReceive(rx_sb, buf, sizeof buf, portMAX_DELAY);
    rtos_print("echo: %.*s\r\n", (int)n, (const char *)buf);
  }
}

int main(void)
{
  board_init();
  rtos_log_init();
  rx_sb = xStreamBufferCreate(64, 1);
  HAL_NVIC_SetPriority(USART1_IRQn, 6, 0);     /* numerically >= configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY (5) */
  HAL_NVIC_EnableIRQ(USART1_IRQn);
  HAL_UART_Receive_IT(&board_uart, &rx_byte, 1);
  xTaskCreate(hello_task, "hello", 256, NULL, 1, NULL);
  xTaskCreate(echo_task, "echo", 256, NULL, 2, NULL);
  vTaskStartScheduler();
  for (;;) {}
}
