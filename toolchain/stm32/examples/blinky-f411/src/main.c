/* WeAct BlackPill STM32F411CEU6: LED on PC13 (active low), HSE 25 MHz, 100 MHz SYSCLK */
#include "clock_f411.h"

void SysTick_Handler(void) { HAL_IncTick(); }

int main(void)
{
  HAL_Init();
  SystemClock_Config100();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  GPIO_InitTypeDef g = { .Pin = GPIO_PIN_13, .Mode = GPIO_MODE_OUTPUT_PP, .Pull = GPIO_NOPULL, .Speed = GPIO_SPEED_FREQ_LOW };
  HAL_GPIO_Init(GPIOC, &g);
  for (;;) { HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13); HAL_Delay(500); }
}
