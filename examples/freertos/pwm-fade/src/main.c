/* pwm-fade: TIM3 channel 1 on PA6 drives an external LED with 1 kHz PWM. A "fade" task walks the duty cycle along a triangle
 * (0 -> 100 % -> 0 in 2 s) with vTaskDelayUntil every 10 ms; a lower-priority task logs the duty once per second.
 * PA6 = TIM3_CH1: AF2 on F411/G431/H743, default (no remap) on F103. On-board LEDs are not on a timer channel. */
#include "board.h"
#include "FreeRTOS.h"
#include "task.h"
#include "rtos_glue.h"

static TIM_HandleTypeDef htim3;
static volatile uint32_t duty_permille;

static void pwm_init(void)
{
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_TIM3_CLK_ENABLE();
  GPIO_InitTypeDef g = {0};
  g.Pin = GPIO_PIN_6; g.Mode = GPIO_MODE_AF_PP; g.Pull = GPIO_NOPULL; g.Speed = GPIO_SPEED_FREQ_LOW;
#if !defined(BOARD_F1)
  g.Alternate = GPIO_AF2_TIM3;
#endif
  HAL_GPIO_Init(GPIOA, &g);
  htim3.Instance = TIM3;
  htim3.Init.Prescaler = BOARD_TIM_APB1_HZ / 1000000U - 1U;   /* 1 MHz counter */
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 1000U - 1U;                              /* 1 kHz PWM, compare = duty in permille */
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_PWM_Init(&htim3) != HAL_OK) for (;;);
  TIM_OC_InitTypeDef oc = {0};
  oc.OCMode = TIM_OCMODE_PWM1; oc.Pulse = 0; oc.OCPolarity = TIM_OCPOLARITY_HIGH; oc.OCFastMode = TIM_OCFAST_DISABLE;
  HAL_TIM_PWM_ConfigChannel(&htim3, &oc, TIM_CHANNEL_1);
  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
}

static void fade_task(void *arg)
{
  (void)arg;
  int duty = 0, step = 10;
  TickType_t last = xTaskGetTickCount();
  for (;;) {
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, (uint32_t)duty);
    duty_permille = (uint32_t)duty;
    duty += step;
    if (duty >= 1000) { duty = 1000; step = -step; } else if (duty <= 0) { duty = 0; step = -step; }
    vTaskDelayUntil(&last, pdMS_TO_TICKS(10));
  }
}

static void log_task(void *arg)
{
  (void)arg;
  for (;;) {
    vTaskDelay(pdMS_TO_TICKS(1000));
    rtos_log("PA6 duty %lu.%lu %%", (unsigned long)(duty_permille / 10), (unsigned long)(duty_permille % 10));
    board_led_toggle();
  }
}

int main(void)
{
  board_init();
  rtos_log_init();
  pwm_init();
  rtos_print("\r\nfreertos pwm-fade on %s: TIM3_CH1 PA6, 1 kHz\r\n", BOARD_NAME);
  xTaskCreate(fade_task, "fade", 128, NULL, 2, NULL);
  xTaskCreate(log_task, "log", 256, NULL, 1, NULL);
  vTaskStartScheduler();
  for (;;) {}
}
