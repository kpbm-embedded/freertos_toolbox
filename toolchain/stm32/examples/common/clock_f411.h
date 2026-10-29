#pragma once
#include "stm32f4xx_hal.h"
/* HSE 25 MHz -> PLL -> 100 MHz SYSCLK, APB1 50, APB2 100 */
static inline void SystemClock_Config100(void)
{
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
  RCC_OscInitTypeDef o = {0};
  o.OscillatorType = RCC_OSCILLATORTYPE_HSE; o.HSEState = RCC_HSE_ON;
  o.PLL.PLLState = RCC_PLL_ON; o.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  o.PLL.PLLM = 25; o.PLL.PLLN = 200; o.PLL.PLLP = RCC_PLLP_DIV2; o.PLL.PLLQ = 9;
  if (HAL_RCC_OscConfig(&o) != HAL_OK) for(;;);
  RCC_ClkInitTypeDef c = {0};
  c.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK|RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  c.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK; c.AHBCLKDivider = RCC_SYSCLK_DIV1;
  c.APB1CLKDivider = RCC_HCLK_DIV2; c.APB2CLKDivider = RCC_HCLK_DIV1;
  if (HAL_RCC_ClockConfig(&c, FLASH_LATENCY_3) != HAL_OK) for(;;);
}
