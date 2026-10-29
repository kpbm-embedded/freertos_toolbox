/* i2c-scan: a "scan" task probes every 7-bit address 0x08..0x77 on I2C1 (100 kHz) with HAL_I2C_IsDeviceReady, prints an
 * i2cdetect-style table, then sleeps 5 s (vTaskDelay, so the CPU is free meanwhile). An LED task blinks while it runs.
 * Pins (I2C1, external 4.7k pull-ups recommended): F411 and H743 PB8 SCL / PB9 SDA (AF4); G431 PA15 SCL / PB7 SDA (AF4, PB8 is
 * BOOT0 on the G431); F103 PB6 SCL / PB7 SDA (no remap). G431/H743 run I2C1 from HSI (16 / 64 MHz) with a fixed TIMINGR value. */
#include "board.h"
#include "FreeRTOS.h"
#include "task.h"
#include "rtos_glue.h"

static I2C_HandleTypeDef hi2c;

static int i2c_init(void)
{
  GPIO_InitTypeDef g = {0};
  g.Mode = GPIO_MODE_AF_OD; g.Pull = GPIO_PULLUP; g.Speed = GPIO_SPEED_FREQ_HIGH;
#if defined(BOARD_F1)
  __HAL_RCC_GPIOB_CLK_ENABLE();
  g.Pin = GPIO_PIN_6 | GPIO_PIN_7; g.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &g);
#elif defined(BOARD_G4)
  __HAL_RCC_GPIOA_CLK_ENABLE(); __HAL_RCC_GPIOB_CLK_ENABLE();
  g.Alternate = GPIO_AF4_I2C1;
  g.Pin = GPIO_PIN_15; HAL_GPIO_Init(GPIOA, &g);
  g.Pin = GPIO_PIN_7;  HAL_GPIO_Init(GPIOB, &g);
  RCC_PeriphCLKInitTypeDef pc = { .PeriphClockSelection = RCC_PERIPHCLK_I2C1, .I2c1ClockSelection = RCC_I2C1CLKSOURCE_HSI };
  HAL_RCCEx_PeriphCLKConfig(&pc);
#elif defined(BOARD_H7)
  __HAL_RCC_GPIOB_CLK_ENABLE();
  g.Alternate = GPIO_AF4_I2C1; g.Pin = GPIO_PIN_8 | GPIO_PIN_9;
  HAL_GPIO_Init(GPIOB, &g);
  RCC_PeriphCLKInitTypeDef pc = {0};
  pc.PeriphClockSelection = RCC_PERIPHCLK_I2C123; pc.I2c123ClockSelection = RCC_I2C123CLKSOURCE_HSI;
  HAL_RCCEx_PeriphCLKConfig(&pc);
#else
  __HAL_RCC_GPIOB_CLK_ENABLE();
  g.Alternate = GPIO_AF4_I2C1; g.Pin = GPIO_PIN_8 | GPIO_PIN_9;
  HAL_GPIO_Init(GPIOB, &g);
#endif
  __HAL_RCC_I2C1_CLK_ENABLE();
  hi2c.Instance = I2C1;
#if defined(BOARD_G4)
  hi2c.Init.Timing = 0x00303D5BU;            /* 16 MHz I2CCLK, 100 kHz standard mode (RM0440 timing example values) */
#elif defined(BOARD_H7)
  hi2c.Init.Timing = 0x30303D5BU;            /* 64 MHz HSI, PRESC=3 -> 16 MHz, then the same 100 kHz values */
#else
  hi2c.Init.ClockSpeed = 100000; hi2c.Init.DutyCycle = I2C_DUTYCYCLE_2;
#endif
  hi2c.Init.OwnAddress1 = 0; hi2c.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE; hi2c.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  return HAL_I2C_Init(&hi2c) == HAL_OK;
}

static void scan_task(void *arg)
{
  (void)arg;
  if (!i2c_init()) { rtos_log("I2C1 init failed"); vTaskDelete(NULL); }
  for (;;) {
    int found = 0;
    rtos_print("\r\n     0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f");
    for (uint8_t a = 0; a < 0x78; a++) {
      if ((a & 0xf) == 0) rtos_print("\r\n%02x:", a);
      if (a < 0x08) { rtos_print("   "); continue; }
      if (HAL_I2C_IsDeviceReady(&hi2c, (uint16_t)(a << 1), 1, 5) == HAL_OK) { rtos_print(" %02x", a); found++; }
      else rtos_print(" --");
    }
    rtos_print("\r\n");
    rtos_log("%d device(s) on I2C1", found);
    vTaskDelay(pdMS_TO_TICKS(5000));
  }
}

static void led_task(void *arg) { (void)arg; for (;;) { board_led_toggle(); vTaskDelay(pdMS_TO_TICKS(250)); } }

int main(void)
{
  board_init();
  rtos_log_init();
  rtos_print("\r\nfreertos i2c-scan on %s\r\n", BOARD_NAME);
  xTaskCreate(scan_task, "scan", 384, NULL, 2, NULL);
  xTaskCreate(led_task, "led", configMINIMAL_STACK_SIZE, NULL, 1, NULL);
  vTaskStartScheduler();
  for (;;) {}
}
