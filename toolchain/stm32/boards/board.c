/* Shared board support: clock tree, LED, button (EXTI), console USART1 (PA9/PA10) with printf retarget. */
#include <string.h>
#include "board.h"

UART_HandleTypeDef board_uart;

#if defined(BOARD_F4)
void board_clock_init(void)
{
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
  RCC_OscInitTypeDef o = {0};
  o.OscillatorType = RCC_OSCILLATORTYPE_HSE; o.HSEState = RCC_HSE_ON;
  o.PLL.PLLState = RCC_PLL_ON; o.PLL.PLLSource = RCC_PLLSOURCE_HSE;
#ifdef BOARD_USB48   /* HSE 25 -> /25 -> x384 -> /4 = 96 MHz SYSCLK, /8 = 48 MHz USB */
  o.PLL.PLLM = 25; o.PLL.PLLN = 384; o.PLL.PLLP = RCC_PLLP_DIV4; o.PLL.PLLQ = 8;
#else                /* HSE 25 -> /25 -> x200 -> /2 = 100 MHz SYSCLK */
  o.PLL.PLLM = 25; o.PLL.PLLN = 200; o.PLL.PLLP = RCC_PLLP_DIV2; o.PLL.PLLQ = 9;
#endif
  if (HAL_RCC_OscConfig(&o) != HAL_OK) for (;;);
  RCC_ClkInitTypeDef c = {0};
  c.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
  c.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK; c.AHBCLKDivider = RCC_SYSCLK_DIV1;
  c.APB1CLKDivider = RCC_HCLK_DIV2; c.APB2CLKDivider = RCC_HCLK_DIV1;
  if (HAL_RCC_ClockConfig(&c, FLASH_LATENCY_3) != HAL_OK) for (;;);
}
#elif defined(BOARD_G4)
void board_clock_init(void)   /* HSE 8 -> /2 -> x72 = 288 -> /2 = 144 MHz (range 1 normal, <=150), Q=/6 = 48 MHz */
{
  HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1);
  RCC_OscInitTypeDef o = {0};
  o.OscillatorType = RCC_OSCILLATORTYPE_HSE; o.HSEState = RCC_HSE_ON;
  o.PLL.PLLState = RCC_PLL_ON; o.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  o.PLL.PLLM = RCC_PLLM_DIV2; o.PLL.PLLN = 72; o.PLL.PLLP = RCC_PLLP_DIV2; o.PLL.PLLQ = RCC_PLLQ_DIV6; o.PLL.PLLR = RCC_PLLR_DIV2;
  if (HAL_RCC_OscConfig(&o) != HAL_OK) for (;;);
  RCC_ClkInitTypeDef c = {0};
  c.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
  c.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK; c.AHBCLKDivider = RCC_SYSCLK_DIV1;
  c.APB1CLKDivider = RCC_HCLK_DIV1; c.APB2CLKDivider = RCC_HCLK_DIV1;
  if (HAL_RCC_ClockConfig(&c, FLASH_LATENCY_6) != HAL_OK) for (;;);
}
#elif defined(BOARD_H7)
void board_clock_init(void)   /* HSE 25 -> /5 -> x160 = 800 VCO -> /2 = 400 MHz; AHB/APB /2 (200/100); LDO supply, VOS1 */
{
  HAL_PWREx_ConfigSupply(PWR_LDO_SUPPLY);
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
  while (!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {}
  RCC_OscInitTypeDef o = {0};
  o.OscillatorType = RCC_OSCILLATORTYPE_HSE; o.HSEState = RCC_HSE_ON;
  o.PLL.PLLState = RCC_PLL_ON; o.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  o.PLL.PLLM = 5; o.PLL.PLLN = 160; o.PLL.PLLP = 2; o.PLL.PLLQ = 4; o.PLL.PLLR = 2;
  o.PLL.PLLRGE = RCC_PLL1VCIRANGE_2; o.PLL.PLLVCOSEL = RCC_PLL1VCOWIDE; o.PLL.PLLFRACN = 0;
  if (HAL_RCC_OscConfig(&o) != HAL_OK) for (;;);
  RCC_ClkInitTypeDef c = {0};
  c.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2 |
                RCC_CLOCKTYPE_D3PCLK1 | RCC_CLOCKTYPE_D1PCLK1;
  c.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK; c.SYSCLKDivider = RCC_SYSCLK_DIV1; c.AHBCLKDivider = RCC_HCLK_DIV2;
  c.APB3CLKDivider = RCC_APB3_DIV2; c.APB1CLKDivider = RCC_APB1_DIV2; c.APB2CLKDivider = RCC_APB2_DIV2; c.APB4CLKDivider = RCC_APB4_DIV2;
  if (HAL_RCC_ClockConfig(&c, FLASH_LATENCY_2) != HAL_OK) for (;;);
}
#elif defined(BOARD_F1)
void board_clock_init(void)   /* HSE 8 -> PLL x9 = 72 MHz, APB1 /2 = 36 */
{
  RCC_OscInitTypeDef o = {0};
  o.OscillatorType = RCC_OSCILLATORTYPE_HSE; o.HSEState = RCC_HSE_ON; o.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  o.PLL.PLLState = RCC_PLL_ON; o.PLL.PLLSource = RCC_PLLSOURCE_HSE; o.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&o) != HAL_OK) for (;;);
  RCC_ClkInitTypeDef c = {0};
  c.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
  c.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK; c.AHBCLKDivider = RCC_SYSCLK_DIV1;
  c.APB1CLKDivider = RCC_HCLK_DIV2; c.APB2CLKDivider = RCC_HCLK_DIV1;
  if (HAL_RCC_ClockConfig(&c, FLASH_LATENCY_2) != HAL_OK) for (;;);
}
#endif

void board_led_set(int on)
{
  GPIO_PinState s = (on ^ BOARD_LED_ACTIVE_LOW) ? GPIO_PIN_SET : GPIO_PIN_RESET;
  HAL_GPIO_WritePin(BOARD_LED_PORT, BOARD_LED_PIN, s);
}
void board_led_toggle(void) { HAL_GPIO_TogglePin(BOARD_LED_PORT, BOARD_LED_PIN); }

static void console_init(void)
{
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_USART1_CLK_ENABLE();
  GPIO_InitTypeDef g = {0};
#if defined(BOARD_F1)
  g.Pin = GPIO_PIN_9; g.Mode = GPIO_MODE_AF_PP; g.Speed = GPIO_SPEED_FREQ_HIGH; HAL_GPIO_Init(GPIOA, &g);
  g.Pin = GPIO_PIN_10; g.Mode = GPIO_MODE_INPUT; g.Pull = GPIO_NOPULL; HAL_GPIO_Init(GPIOA, &g);
#else
  g.Pin = GPIO_PIN_9 | GPIO_PIN_10; g.Mode = GPIO_MODE_AF_PP; g.Pull = GPIO_PULLUP;
  g.Speed = GPIO_SPEED_FREQ_HIGH; g.Alternate = GPIO_AF7_USART1;
  HAL_GPIO_Init(GPIOA, &g);
#endif
  board_uart.Instance = USART1;
  board_uart.Init.BaudRate = 115200; board_uart.Init.WordLength = UART_WORDLENGTH_8B;
  board_uart.Init.StopBits = UART_STOPBITS_1; board_uart.Init.Parity = UART_PARITY_NONE;
  board_uart.Init.Mode = UART_MODE_TX_RX; board_uart.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  board_uart.Init.OverSampling = UART_OVERSAMPLING_16;
  HAL_UART_Init(&board_uart);
  setvbuf(stdout, NULL, _IONBF, 0);
}

void board_init(void)
{
  HAL_Init();
  board_clock_init();
  BOARD_LED_CLK_EN();
  GPIO_InitTypeDef g = {0};
  g.Pin = BOARD_LED_PIN; g.Mode = GPIO_MODE_OUTPUT_PP; g.Pull = GPIO_NOPULL; g.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(BOARD_LED_PORT, &g);
  board_led_set(0);
  console_init();
}

int _write(int fd, char *buf, int len)
{
  (void)fd;
  HAL_UART_Transmit(&board_uart, (uint8_t *)buf, (uint16_t)len, 1000);
  return len;
}

#ifndef BOARD_NO_SYSTICK
void SysTick_Handler(void) { HAL_IncTick(); }
#endif

#if BOARD_HAS_BUTTON
__attribute__((weak)) void board_button_hook(void) {}

void board_button_init(int use_irq)
{
  BOARD_BTN_CLK_EN();
  GPIO_InitTypeDef g = {0};
  g.Pin = BOARD_BTN_PIN; g.Pull = BOARD_BTN_PULL; g.Speed = GPIO_SPEED_FREQ_LOW;
  g.Mode = !use_irq ? GPIO_MODE_INPUT : (BOARD_BTN_ACTIVE_LOW ? GPIO_MODE_IT_FALLING : GPIO_MODE_IT_RISING);
  HAL_GPIO_Init(BOARD_BTN_PORT, &g);
  if (use_irq) { HAL_NVIC_SetPriority(BOARD_BTN_IRQn, 6, 0); HAL_NVIC_EnableIRQ(BOARD_BTN_IRQn); }
}
int board_button_pressed(void)
{
  return (HAL_GPIO_ReadPin(BOARD_BTN_PORT, BOARD_BTN_PIN) == GPIO_PIN_SET) ^ BOARD_BTN_ACTIVE_LOW;
}
void BOARD_BTN_IRQ_HANDLER(void) { HAL_GPIO_EXTI_IRQHandler(BOARD_BTN_PIN); }
void HAL_GPIO_EXTI_Callback(uint16_t pin) { if (pin == BOARD_BTN_PIN) board_button_hook(); }   /* same name on F1/F4/G4/H7 in the pinned Cube packs */
#endif
