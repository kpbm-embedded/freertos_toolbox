/* Common board API (stm32/boards). Per-board macros come from <board>/board.h, which includes this file last. */
#pragma once
#include <stdint.h>
#include <stdio.h>
#include BOARD_HAL_HEADER

extern UART_HandleTypeDef board_uart;   /* console: USART1 TX=PA9 RX=PA10, 115200 8N1; printf() goes here */

void board_init(void);                  /* HAL_Init + clock tree + LED + console UART */
void board_clock_init(void);
void board_led_set(int on);
void board_led_toggle(void);
#if BOARD_HAS_BUTTON
void board_button_init(int use_irq);    /* input with pull; if use_irq: EXTI on the press edge + NVIC enabled */
int  board_button_pressed(void);
void board_button_hook(void);           /* weak: define it in the app, called from the EXTI ISR */
#endif
