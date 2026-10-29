#pragma once
#define BOARD_NAME "bluepill-f103"
#define BOARD_HAL_HEADER "stm32f1xx_hal.h"
#define BOARD_SYSCLK_HZ 72000000U
#define BOARD_TIM_APB1_HZ 72000000U            /* APB1 36 MHz (/2) -> timers x2 */
#define BOARD_TIM_APB2_HZ 72000000U
#define BOARD_LED_PORT GPIOC
#define BOARD_LED_PIN GPIO_PIN_13
#define BOARD_LED_ACTIVE_LOW 1
#define BOARD_LED_CLK_EN() __HAL_RCC_GPIOC_CLK_ENABLE()
#define BOARD_HAS_BUTTON 0
#include "board_common.h"
