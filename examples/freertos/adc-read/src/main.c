/* adc-read: a task starts one ADC1 conversion of PA1 every 500 ms with HAL_ADC_Start_IT; the end-of-conversion interrupt
 * (priority 6) stores the sample and notifies the task, which converts it to millivolts (VDDA assumed 3.3 V) and logs it.
 * PA1 is ADC1_IN1 on F411/F103, ADC1_IN2 on G431 and ADC1_INP17 on H743 (16-bit there). */
#include "board.h"
#include "FreeRTOS.h"
#include "task.h"
#include "rtos_glue.h"

static ADC_HandleTypeDef hadc;
static TaskHandle_t adc_h;
static volatile uint32_t sample;

#if defined(BOARD_H7)
#define ADC_FULL_SCALE 65535U
#define PA1_CHANNEL    ADC_CHANNEL_17
#define ADC_IRQ        ADC_IRQn
#elif defined(BOARD_G4)
#define ADC_FULL_SCALE 4095U
#define PA1_CHANNEL    ADC_CHANNEL_2
#define ADC_IRQ        ADC1_2_IRQn
#elif defined(BOARD_F1)
#define ADC_FULL_SCALE 4095U
#define PA1_CHANNEL    ADC_CHANNEL_1
#define ADC_IRQ        ADC1_2_IRQn
#else
#define ADC_FULL_SCALE 4095U
#define PA1_CHANNEL    ADC_CHANNEL_1
#define ADC_IRQ        ADC_IRQn
#endif

#if defined(BOARD_F4)
void ADC_IRQHandler(void) { HAL_ADC_IRQHandler(&hadc); }
#elif defined(BOARD_H7)
void ADC_IRQHandler(void) { HAL_ADC_IRQHandler(&hadc); }
#else
void ADC1_2_IRQHandler(void) { HAL_ADC_IRQHandler(&hadc); }
#endif
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *h)
{
  BaseType_t woken = pdFALSE;
  sample = HAL_ADC_GetValue(h);
  vTaskNotifyGiveFromISR(adc_h, &woken);
  portYIELD_FROM_ISR(woken);
}

static void adc_init(void)
{
  __HAL_RCC_GPIOA_CLK_ENABLE();
  GPIO_InitTypeDef g = { .Pin = GPIO_PIN_1, .Mode = GPIO_MODE_ANALOG, .Pull = GPIO_NOPULL };
  HAL_GPIO_Init(GPIOA, &g);
  ADC_ChannelConfTypeDef c = {0};
  hadc.Instance = ADC1;
#if defined(BOARD_F1)
  RCC_PeriphCLKInitTypeDef pc = { .PeriphClockSelection = RCC_PERIPHCLK_ADC, .AdcClockSelection = RCC_ADCPCLK2_DIV6 };   /* 72/6 = 12 MHz (max 14) */
  HAL_RCCEx_PeriphCLKConfig(&pc);
  __HAL_RCC_ADC1_CLK_ENABLE();
  hadc.Init.ScanConvMode = ADC_SCAN_DISABLE; hadc.Init.ContinuousConvMode = DISABLE; hadc.Init.DiscontinuousConvMode = DISABLE;
  hadc.Init.ExternalTrigConv = ADC_SOFTWARE_START; hadc.Init.DataAlign = ADC_DATAALIGN_RIGHT; hadc.Init.NbrOfConversion = 1;
  if (HAL_ADC_Init(&hadc) != HAL_OK) for (;;);
  HAL_ADCEx_Calibration_Start(&hadc);
  c.Channel = PA1_CHANNEL; c.Rank = ADC_REGULAR_RANK_1; c.SamplingTime = ADC_SAMPLETIME_239CYCLES_5;
#elif defined(BOARD_F4)
  __HAL_RCC_ADC1_CLK_ENABLE();
  hadc.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;        /* PCLK2 100 MHz / 4 = 25 MHz (max 36) */
  hadc.Init.Resolution = ADC_RESOLUTION_12B;
  hadc.Init.ScanConvMode = DISABLE; hadc.Init.ContinuousConvMode = DISABLE; hadc.Init.DiscontinuousConvMode = DISABLE;
  hadc.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE; hadc.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc.Init.DataAlign = ADC_DATAALIGN_RIGHT; hadc.Init.NbrOfConversion = 1; hadc.Init.DMAContinuousRequests = DISABLE;
  hadc.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  if (HAL_ADC_Init(&hadc) != HAL_OK) for (;;);
  c.Channel = PA1_CHANNEL; c.Rank = 1; c.SamplingTime = ADC_SAMPLETIME_144CYCLES;
#elif defined(BOARD_G4)
  __HAL_RCC_ADC12_CLK_ENABLE();
  hadc.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;        /* HCLK 144 MHz / 4 = 36 MHz */
  hadc.Init.Resolution = ADC_RESOLUTION_12B; hadc.Init.DataAlign = ADC_DATAALIGN_RIGHT; hadc.Init.GainCompensation = 0;
  hadc.Init.ScanConvMode = ADC_SCAN_DISABLE; hadc.Init.EOCSelection = ADC_EOC_SINGLE_CONV; hadc.Init.LowPowerAutoWait = DISABLE;
  hadc.Init.ContinuousConvMode = DISABLE; hadc.Init.NbrOfConversion = 1; hadc.Init.DiscontinuousConvMode = DISABLE;
  hadc.Init.ExternalTrigConv = ADC_SOFTWARE_START; hadc.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc.Init.DMAContinuousRequests = DISABLE; hadc.Init.Overrun = ADC_OVR_DATA_OVERWRITTEN; hadc.Init.OversamplingMode = DISABLE;
  if (HAL_ADC_Init(&hadc) != HAL_OK) for (;;);
  HAL_ADCEx_Calibration_Start(&hadc, ADC_SINGLE_ENDED);
  c.Channel = PA1_CHANNEL; c.Rank = ADC_REGULAR_RANK_1; c.SamplingTime = ADC_SAMPLETIME_247CYCLES_5;
  c.SingleDiff = ADC_SINGLE_ENDED; c.OffsetNumber = ADC_OFFSET_NONE; c.Offset = 0;
#elif defined(BOARD_H7)
  __HAL_RCC_ADC12_CLK_ENABLE();
  hadc.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;        /* HCLK 200 MHz / 4 = 50 MHz (HAL sets the boost bits) */
  hadc.Init.Resolution = ADC_RESOLUTION_16B; hadc.Init.ScanConvMode = ADC_SCAN_DISABLE; hadc.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  hadc.Init.LowPowerAutoWait = DISABLE; hadc.Init.ContinuousConvMode = DISABLE; hadc.Init.NbrOfConversion = 1;
  hadc.Init.DiscontinuousConvMode = DISABLE; hadc.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE; hadc.Init.ConversionDataManagement = ADC_CONVERSIONDATA_DR;
  hadc.Init.Overrun = ADC_OVR_DATA_OVERWRITTEN; hadc.Init.LeftBitShift = ADC_LEFTBITSHIFT_NONE; hadc.Init.OversamplingMode = DISABLE;
  if (HAL_ADC_Init(&hadc) != HAL_OK) for (;;);
  HAL_ADCEx_Calibration_Start(&hadc, ADC_CALIB_OFFSET, ADC_SINGLE_ENDED);
  c.Channel = PA1_CHANNEL; c.Rank = ADC_REGULAR_RANK_1; c.SamplingTime = ADC_SAMPLETIME_387CYCLES_5;
  c.SingleDiff = ADC_SINGLE_ENDED; c.OffsetNumber = ADC_OFFSET_NONE; c.Offset = 0; c.OffsetSignedSaturation = DISABLE;
#endif
  if (HAL_ADC_ConfigChannel(&hadc, &c) != HAL_OK) for (;;);
  HAL_NVIC_SetPriority(ADC_IRQ, 6, 0);
  HAL_NVIC_EnableIRQ(ADC_IRQ);
}

static void adc_task(void *arg)
{
  (void)arg;
  TickType_t last = xTaskGetTickCount();
  for (;;) {
    HAL_ADC_Start_IT(&hadc);
    if (ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(10)) == 0) {
      rtos_log("ADC timeout");
    } else {
      uint32_t raw = sample, mv = raw * 3300U / ADC_FULL_SCALE;
      rtos_log("PA1 raw=%5lu  %4lu mV", (unsigned long)raw, (unsigned long)mv);
    }
    board_led_toggle();
    vTaskDelayUntil(&last, pdMS_TO_TICKS(500));
  }
}

int main(void)
{
  board_init();
  rtos_log_init();
  adc_init();
  rtos_print("\r\nfreertos adc-read on %s: PA1, end-of-conversion IRQ -> task\r\n", BOARD_NAME);
  xTaskCreate(adc_task, "adc", 256, NULL, 2, &adc_h);
  vTaskStartScheduler();
  for (;;) {}
}
