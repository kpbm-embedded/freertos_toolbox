/* dma-uart: a console "server" task owns USART1 TX and sends every line with HAL_UART_Transmit_DMA. Two producer tasks post
 * formatted lines into a FreeRTOS queue; the server takes one, starts the DMA transfer and blocks on a task notification that
 * HAL_UART_TxCpltCallback (USART1 TC interrupt, priority 6) gives when the last byte has left. The CPU is free during transfers.
 * USART1_TX DMA: F411 DMA2 Stream7 channel 4; F103 DMA1 Channel4; G431 DMA1 Channel1 + DMAMUX request USART1_TX;
 * H743 DMA1 Stream0 + DMAMUX request USART1_TX (buffers in AXI SRAM, D-cache off in this board layer). */
#include <stdio.h>
#include <string.h>
#include "board.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

typedef struct { char text[64]; } line_t;

static DMA_HandleTypeDef hdma_tx;
static QueueHandle_t lines;
static TaskHandle_t server_h;
static line_t txbuf;                       /* the DMA reads from here; only the server task writes it */

#if defined(BOARD_F4)
#define TX_DMA_IRQn DMA2_Stream7_IRQn
void DMA2_Stream7_IRQHandler(void) { HAL_DMA_IRQHandler(&hdma_tx); }
#elif defined(BOARD_F1)
#define TX_DMA_IRQn DMA1_Channel4_IRQn
void DMA1_Channel4_IRQHandler(void) { HAL_DMA_IRQHandler(&hdma_tx); }
#elif defined(BOARD_G4)
#define TX_DMA_IRQn DMA1_Channel1_IRQn
void DMA1_Channel1_IRQHandler(void) { HAL_DMA_IRQHandler(&hdma_tx); }
#elif defined(BOARD_H7)
#define TX_DMA_IRQn DMA1_Stream0_IRQn
void DMA1_Stream0_IRQHandler(void) { HAL_DMA_IRQHandler(&hdma_tx); }
#endif
void USART1_IRQHandler(void) { HAL_UART_IRQHandler(&board_uart); }

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *h)
{
  BaseType_t woken = pdFALSE;
  if (h == &board_uart) vTaskNotifyGiveFromISR(server_h, &woken);
  portYIELD_FROM_ISR(woken);
}

static void dma_init(void)
{
#if defined(BOARD_F4)
  __HAL_RCC_DMA2_CLK_ENABLE();
  hdma_tx.Instance = DMA2_Stream7;
  hdma_tx.Init.Channel = DMA_CHANNEL_4;
  hdma_tx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
#elif defined(BOARD_F1)
  __HAL_RCC_DMA1_CLK_ENABLE();
  hdma_tx.Instance = DMA1_Channel4;
#elif defined(BOARD_G4)
  __HAL_RCC_DMAMUX1_CLK_ENABLE();
  __HAL_RCC_DMA1_CLK_ENABLE();
  hdma_tx.Instance = DMA1_Channel1;
  hdma_tx.Init.Request = DMA_REQUEST_USART1_TX;
#elif defined(BOARD_H7)
  __HAL_RCC_DMA1_CLK_ENABLE();
  hdma_tx.Instance = DMA1_Stream0;
  hdma_tx.Init.Request = DMA_REQUEST_USART1_TX;
  hdma_tx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
#endif
  hdma_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
  hdma_tx.Init.PeriphInc = DMA_PINC_DISABLE;
  hdma_tx.Init.MemInc = DMA_MINC_ENABLE;
  hdma_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
  hdma_tx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
  hdma_tx.Init.Mode = DMA_NORMAL;
  hdma_tx.Init.Priority = DMA_PRIORITY_LOW;
  if (HAL_DMA_Init(&hdma_tx) != HAL_OK) for (;;);
  __HAL_LINKDMA(&board_uart, hdmatx, hdma_tx);
  HAL_NVIC_SetPriority(TX_DMA_IRQn, 6, 0);
  HAL_NVIC_EnableIRQ(TX_DMA_IRQn);
  HAL_NVIC_SetPriority(USART1_IRQn, 6, 0);
  HAL_NVIC_EnableIRQ(USART1_IRQn);
}

static void post(const char *fmt, unsigned a, unsigned b)
{
  line_t l;
  snprintf(l.text, sizeof l.text, fmt, a, b);
  xQueueSend(lines, &l, pdMS_TO_TICKS(100));
}

static void server_task(void *arg)
{
  (void)arg;
  uint32_t sent = 0;
  line_t l;
  for (;;) {
    xQueueReceive(lines, &l, portMAX_DELAY);
    memcpy(&txbuf, &l, sizeof txbuf);
    size_t n = strnlen(txbuf.text, sizeof txbuf.text);
    if (HAL_UART_Transmit_DMA(&board_uart, (uint8_t *)txbuf.text, (uint16_t)n) == HAL_OK &&
        ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(100)) != 0)
      sent += n;
    if ((sent & 0x3ffU) < n) board_led_toggle();       /* blink about once per KiB sent */
  }
}

static void fast_task(void *arg)
{
  (void)arg;
  for (unsigned n = 0;; n++) { post("[fast] line %u, tick %u\r\n", n, (unsigned)xTaskGetTickCount()); vTaskDelay(pdMS_TO_TICKS(250)); }
}
static void slow_task(void *arg)
{
  (void)arg;
  for (unsigned n = 0;; n++) {
    post("[slow] line %u, free heap %u B\r\n", n, (unsigned)xPortGetFreeHeapSize());
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

int main(void)
{
  board_init();
  dma_init();
  lines = xQueueCreate(8, sizeof(line_t));
  xTaskCreate(server_task, "server", 256, NULL, 3, &server_h);
  xTaskCreate(fast_task, "fast", 256, NULL, 2, NULL);
  xTaskCreate(slow_task, "slow", 256, NULL, 1, NULL);
  post("\r\nfreertos dma-uart on " BOARD_NAME ": USART1 TX by DMA (%u-byte lines, queue %u)\r\n", (unsigned)sizeof(line_t), 8U);
  vTaskStartScheduler();
  for (;;) {}
}
