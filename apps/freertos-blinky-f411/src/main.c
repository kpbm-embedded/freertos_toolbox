/* BlackPill F411: task 1 blinks PC13 (active low), task 2 prints on USART1 TX (PA9) at 115200. */
#include <stdio.h>
#include "FreeRTOS.h"
#include "task.h"
#include "clock_f411.h"

static UART_HandleTypeDef huart1;

/* HAL timebase: SysTick belongs to FreeRTOS. Before the scheduler runs use the DWT cycle counter. */
HAL_StatusTypeDef HAL_InitTick(uint32_t p) { (void)p; CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk; DWT->CYCCNT = 0; DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk; return HAL_OK; }
uint32_t HAL_GetTick(void)
{
  if (xTaskGetSchedulerState() == taskSCHEDULER_NOT_STARTED) return DWT->CYCCNT / (SystemCoreClock / 1000U);
  return xTaskGetTickCount();
}

int _write(int fd, char *buf, int len) { (void)fd; HAL_UART_Transmit(&huart1, (uint8_t *)buf, len, 1000); return len; }

static void led_task(void *a)
{
  (void)a;
  for (;;) { HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13); vTaskDelay(pdMS_TO_TICKS(500)); }
}
static void uart_task(void *a)
{
  (void)a;
  for (unsigned n = 0;; n++) { printf("FreeRTOS tick=%lu hello #%u\r\n", (unsigned long)xTaskGetTickCount(), n); vTaskDelay(pdMS_TO_TICKS(1000)); }
}
void vApplicationMallocFailedHook(void) { for (;;); }
void vApplicationStackOverflowHook(TaskHandle_t t, char *n) { (void)t; (void)n; for (;;); }

int main(void)
{
  HAL_Init();
  SystemClock_Config100();
  __HAL_RCC_GPIOC_CLK_ENABLE(); __HAL_RCC_GPIOA_CLK_ENABLE(); __HAL_RCC_USART1_CLK_ENABLE();
  GPIO_InitTypeDef g = { .Pin = GPIO_PIN_13, .Mode = GPIO_MODE_OUTPUT_PP, .Speed = GPIO_SPEED_FREQ_LOW };
  HAL_GPIO_Init(GPIOC, &g);
  g = (GPIO_InitTypeDef){ .Pin = GPIO_PIN_9, .Mode = GPIO_MODE_AF_PP, .Pull = GPIO_PULLUP, .Speed = GPIO_SPEED_FREQ_HIGH, .Alternate = GPIO_AF7_USART1 };
  HAL_GPIO_Init(GPIOA, &g);
  huart1.Instance = USART1; huart1.Init.BaudRate = 115200; huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1; huart1.Init.Parity = UART_PARITY_NONE; huart1.Init.Mode = UART_MODE_TX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE; huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  HAL_UART_Init(&huart1);
  setvbuf(stdout, NULL, _IONBF, 0);
  xTaskCreate(led_task, "led", 128, NULL, 1, NULL);
  xTaskCreate(uart_task, "uart", 512, NULL, 1, NULL);
  vTaskStartScheduler();
  for (;;);
}
