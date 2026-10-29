/* timer: hardware timer ISR -> task. TIM2 (10 kHz counter, ARR 999) raises an update interrupt every 100 ms at NVIC priority 6;
 * the ISR only calls vTaskNotifyGiveFromISR. The worker task takes the notifications, toggles the LED every 5 periods (2 Hz blink)
 * and logs the count once per second. Missed periods would show up as a notification count > 1. */
#include "board.h"
#include "FreeRTOS.h"
#include "task.h"
#include "rtos_glue.h"

static TIM_HandleTypeDef htim2;
static TaskHandle_t worker_h;

void TIM2_IRQHandler(void) { HAL_TIM_IRQHandler(&htim2); }
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *h)
{
  BaseType_t woken = pdFALSE;
  if (h->Instance == TIM2 && worker_h) vTaskNotifyGiveFromISR(worker_h, &woken);
  portYIELD_FROM_ISR(woken);
}

static void worker_task(void *arg)
{
  (void)arg;
  uint32_t periods = 0, late = 0;
  for (;;) {
    uint32_t n = ulTaskNotifyTake(pdTRUE, portMAX_DELAY);   /* returns how many ISR gives were pending */
    if (n > 1) late += n - 1;
    for (uint32_t i = 0; i < n; i++) {
      periods++;
      if (periods % 5 == 0) board_led_toggle();
      if (periods % 10 == 0) rtos_log("TIM2: %lu periods of 100 ms, %lu handled late", (unsigned long)periods, (unsigned long)late);
    }
  }
}

static void tim2_start(void)
{
  __HAL_RCC_TIM2_CLK_ENABLE();
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = BOARD_TIM_APB1_HZ / 10000U - 1U;   /* 10 kHz counter clock */
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 1000U - 1U;                            /* 100 ms */
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK) for (;;);
  HAL_NVIC_SetPriority(TIM2_IRQn, 6, 0);                     /* may call FromISR APIs (>= 5) */
  HAL_NVIC_EnableIRQ(TIM2_IRQn);
  HAL_TIM_Base_Start_IT(&htim2);
}

int main(void)
{
  board_init();
  rtos_log_init();
  rtos_print("\r\nfreertos timer on %s: TIM2 100 ms -> task notification\r\n", BOARD_NAME);
  xTaskCreate(worker_task, "worker", 256, NULL, 3, &worker_h);
  tim2_start();                                              /* gives before the scheduler runs are kept as pending notifications */
  vTaskStartScheduler();
  for (;;) {}
}
