/* Low-level glue between the Cube USB Device core and HAL_PCD. Two USB IPs are involved:
 *   OTG_FS (F411, H743): PA11 DM / PA12 DP, AF10; dedicated FIFO RAM sized below. VBUS sensing off (PA9 is the console TX).
 *     F411 48 MHz from PLLQ (BOARD_USB48 -> 96 MHz SYSCLK); H743 48 MHz from HSI48, trimmed by CRS on the USB SOF.
 *   FSDEV "USB" (G431, F103): PA11/PA12 dedicated, packet memory (PMA) laid out below.
 *     G431 48 MHz from PLLQ (board clock tree), internal D+ pull-up. F103 48 MHz = PLL 72 / 1.5; the BluePill has a fixed
 *     external D+ pull-up, so PA12 is pulled low for 10 ms first to make the host re-enumerate after a reset.
 * All USB interrupts run at NVIC priority 6 (>= the FreeRTOS syscall ceiling 5), so callbacks may use FromISR APIs. */
#include "usbd_core.h"
#include "usbd_cdc.h"

PCD_HandleTypeDef hpcd_usb;

#if defined(BOARD_F4) || defined(BOARD_H7)
#define USB_IRQ OTG_FS_IRQn
void OTG_FS_IRQHandler(void) { HAL_PCD_IRQHandler(&hpcd_usb); }
#elif defined(BOARD_G4)
#define USB_IRQ USB_LP_IRQn
void USB_LP_IRQHandler(void) { HAL_PCD_IRQHandler(&hpcd_usb); }
#elif defined(BOARD_F1)
#define USB_IRQ USB_LP_CAN1_RX0_IRQn
void USB_LP_CAN1_RX0_IRQHandler(void) { HAL_PCD_IRQHandler(&hpcd_usb); }
#endif

void HAL_PCD_MspInit(PCD_HandleTypeDef *h)
{
  (void)h;
#if defined(BOARD_F4) || defined(BOARD_H7)
  __HAL_RCC_GPIOA_CLK_ENABLE();
  GPIO_InitTypeDef g = { .Pin = GPIO_PIN_11 | GPIO_PIN_12, .Mode = GPIO_MODE_AF_PP, .Pull = GPIO_NOPULL,
                         .Speed = GPIO_SPEED_FREQ_VERY_HIGH };
#if defined(BOARD_F4)
  g.Alternate = GPIO_AF10_OTG_FS;
  HAL_GPIO_Init(GPIOA, &g);
  __HAL_RCC_USB_OTG_FS_CLK_ENABLE();
#else
  g.Alternate = GPIO_AF10_OTG2_FS;               /* H743: the FS-only OTG on PA11/PA12 is "OTG2" in the HAL */
  HAL_GPIO_Init(GPIOA, &g);
  RCC_OscInitTypeDef o = {0};
  o.OscillatorType = RCC_OSCILLATORTYPE_HSI48; o.HSI48State = RCC_HSI48_ON; o.PLL.PLLState = RCC_PLL_NONE;
  HAL_RCC_OscConfig(&o);
  RCC_PeriphCLKInitTypeDef pc = {0};
  pc.PeriphClockSelection = RCC_PERIPHCLK_USB; pc.UsbClockSelection = RCC_USBCLKSOURCE_HSI48;
  HAL_RCCEx_PeriphCLKConfig(&pc);
  __HAL_RCC_CRS_CLK_ENABLE();
  RCC_CRSInitTypeDef crs = { .Prescaler = RCC_CRS_SYNC_DIV1, .Source = RCC_CRS_SYNC_SOURCE_USB2, .Polarity = RCC_CRS_SYNC_POLARITY_RISING,
                             .ReloadValue = __HAL_RCC_CRS_RELOADVALUE_CALCULATE(48000000U, 1000U), .ErrorLimitValue = 34U,
                             .HSI48CalibrationValue = 32U };
  HAL_RCCEx_CRSConfig(&crs);
  HAL_PWREx_EnableUSBVoltageDetector();
  __HAL_RCC_USB2_OTG_FS_CLK_ENABLE();
#endif
#elif defined(BOARD_G4)
  RCC_PeriphCLKInitTypeDef pc = { .PeriphClockSelection = RCC_PERIPHCLK_USB, .UsbClockSelection = RCC_USBCLKSOURCE_PLL };
  HAL_RCCEx_PeriphCLKConfig(&pc);
  __HAL_RCC_USB_CLK_ENABLE();
#elif defined(BOARD_F1)
  RCC_PeriphCLKInitTypeDef pc = { .PeriphClockSelection = RCC_PERIPHCLK_USB, .UsbClockSelection = RCC_USBCLKSOURCE_PLL_DIV1_5 };
  HAL_RCCEx_PeriphCLKConfig(&pc);
  __HAL_RCC_GPIOA_CLK_ENABLE();
  GPIO_InitTypeDef g = { .Pin = GPIO_PIN_12, .Mode = GPIO_MODE_OUTPUT_PP, .Speed = GPIO_SPEED_FREQ_LOW };
  HAL_GPIO_Init(GPIOA, &g);
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_12, GPIO_PIN_RESET);     /* SE0 -> host sees a disconnect */
  HAL_Delay(10);
  g.Mode = GPIO_MODE_INPUT; g.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &g);                                   /* hand PA12 back to the USB peripheral */
  __HAL_RCC_USB_CLK_ENABLE();
#endif
  HAL_NVIC_SetPriority(USB_IRQ, 6, 0);
  HAL_NVIC_EnableIRQ(USB_IRQ);
}

void HAL_PCD_SetupStageCallback(PCD_HandleTypeDef *h) { USBD_LL_SetupStage(h->pData, (uint8_t *)h->Setup); }
void HAL_PCD_DataOutStageCallback(PCD_HandleTypeDef *h, uint8_t ep) { USBD_LL_DataOutStage(h->pData, ep, h->OUT_ep[ep].xfer_buff); }
void HAL_PCD_DataInStageCallback(PCD_HandleTypeDef *h, uint8_t ep) { USBD_LL_DataInStage(h->pData, ep, h->IN_ep[ep].xfer_buff); }
void HAL_PCD_SOFCallback(PCD_HandleTypeDef *h) { USBD_LL_SOF(h->pData); }
void HAL_PCD_ResetCallback(PCD_HandleTypeDef *h) { USBD_LL_SetSpeed(h->pData, USBD_SPEED_FULL); USBD_LL_Reset(h->pData); }
void HAL_PCD_SuspendCallback(PCD_HandleTypeDef *h) { USBD_LL_Suspend(h->pData); }
void HAL_PCD_ResumeCallback(PCD_HandleTypeDef *h) { USBD_LL_Resume(h->pData); }
void HAL_PCD_ISOOUTIncompleteCallback(PCD_HandleTypeDef *h, uint8_t ep) { USBD_LL_IsoOUTIncomplete(h->pData, ep); }
void HAL_PCD_ISOINIncompleteCallback(PCD_HandleTypeDef *h, uint8_t ep) { USBD_LL_IsoINIncomplete(h->pData, ep); }
void HAL_PCD_ConnectCallback(PCD_HandleTypeDef *h) { USBD_LL_DevConnected(h->pData); }
void HAL_PCD_DisconnectCallback(PCD_HandleTypeDef *h) { USBD_LL_DevDisconnected(h->pData); }

static USBD_StatusTypeDef st(HAL_StatusTypeDef s) { return s == HAL_OK ? USBD_OK : (s == HAL_BUSY ? USBD_BUSY : USBD_FAIL); }

USBD_StatusTypeDef USBD_LL_Init(USBD_HandleTypeDef *pdev)
{
  hpcd_usb.pData = pdev; pdev->pData = &hpcd_usb;
  hpcd_usb.Init.speed = PCD_SPEED_FULL; hpcd_usb.Init.phy_itface = PCD_PHY_EMBEDDED;
  hpcd_usb.Init.Sof_enable = DISABLE; hpcd_usb.Init.low_power_enable = DISABLE; hpcd_usb.Init.lpm_enable = DISABLE;
#if defined(BOARD_F4) || defined(BOARD_H7)
  hpcd_usb.Instance = USB_OTG_FS;
  hpcd_usb.Init.dev_endpoints = 4; hpcd_usb.Init.dma_enable = DISABLE;
  hpcd_usb.Init.vbus_sensing_enable = DISABLE; hpcd_usb.Init.use_dedicated_ep1 = DISABLE;
#if defined(BOARD_H7)
  hpcd_usb.Init.Sof_enable = ENABLE;               /* CRS syncs HSI48 on the SOF */
#endif
  if (HAL_PCD_Init(&hpcd_usb) != HAL_OK) return USBD_FAIL;
  HAL_PCDEx_SetRxFiFo(&hpcd_usb, 0x40);           /* 320 words (F411 FS FIFO): rx 64 + tx0 32 + tx1 128 + tx2 32 */
  HAL_PCDEx_SetTxFiFo(&hpcd_usb, 0, 0x20);
  HAL_PCDEx_SetTxFiFo(&hpcd_usb, 1, 0x80);
  HAL_PCDEx_SetTxFiFo(&hpcd_usb, 2, 0x20);
#else
  hpcd_usb.Instance = USB;
  hpcd_usb.Init.dev_endpoints = 8; hpcd_usb.Init.battery_charging_enable = DISABLE;
  if (HAL_PCD_Init(&hpcd_usb) != HAL_OK) return USBD_FAIL;
  /* PMA: 8-entry buffer table at 0x00, then EP0 OUT/IN 64 B each, CDC command IN 8 B, CDC data OUT/IN 64 B each */
  HAL_PCDEx_PMAConfig(&hpcd_usb, 0x00, PCD_SNG_BUF, 0x18);
  HAL_PCDEx_PMAConfig(&hpcd_usb, 0x80, PCD_SNG_BUF, 0x58);
  HAL_PCDEx_PMAConfig(&hpcd_usb, CDC_CMD_EP, PCD_SNG_BUF, 0x98);
  HAL_PCDEx_PMAConfig(&hpcd_usb, CDC_OUT_EP, PCD_SNG_BUF, 0xA0);
  HAL_PCDEx_PMAConfig(&hpcd_usb, CDC_IN_EP, PCD_SNG_BUF, 0xE0);
#endif
  return USBD_OK;
}
USBD_StatusTypeDef USBD_LL_DeInit(USBD_HandleTypeDef *pdev) { return st(HAL_PCD_DeInit(pdev->pData)); }
USBD_StatusTypeDef USBD_LL_Start(USBD_HandleTypeDef *pdev) { return st(HAL_PCD_Start(pdev->pData)); }
USBD_StatusTypeDef USBD_LL_Stop(USBD_HandleTypeDef *pdev) { return st(HAL_PCD_Stop(pdev->pData)); }
USBD_StatusTypeDef USBD_LL_OpenEP(USBD_HandleTypeDef *pdev, uint8_t ep, uint8_t type, uint16_t mps) { return st(HAL_PCD_EP_Open(pdev->pData, ep, mps, type)); }
USBD_StatusTypeDef USBD_LL_CloseEP(USBD_HandleTypeDef *pdev, uint8_t ep) { return st(HAL_PCD_EP_Close(pdev->pData, ep)); }
USBD_StatusTypeDef USBD_LL_FlushEP(USBD_HandleTypeDef *pdev, uint8_t ep) { return st(HAL_PCD_EP_Flush(pdev->pData, ep)); }
USBD_StatusTypeDef USBD_LL_StallEP(USBD_HandleTypeDef *pdev, uint8_t ep) { return st(HAL_PCD_EP_SetStall(pdev->pData, ep)); }
USBD_StatusTypeDef USBD_LL_ClearStallEP(USBD_HandleTypeDef *pdev, uint8_t ep) { return st(HAL_PCD_EP_ClrStall(pdev->pData, ep)); }
uint8_t USBD_LL_IsStallEP(USBD_HandleTypeDef *pdev, uint8_t ep)
{
  PCD_HandleTypeDef *h = pdev->pData;
  return (ep & 0x80U) ? h->IN_ep[ep & 0x7FU].is_stall : h->OUT_ep[ep & 0x7FU].is_stall;
}
USBD_StatusTypeDef USBD_LL_SetUSBAddress(USBD_HandleTypeDef *pdev, uint8_t addr) { return st(HAL_PCD_SetAddress(pdev->pData, addr)); }
#if defined(BOARD_F1)
typedef uint16_t usbd_size_t;              /* the F1 pack ships USB Device library V2.5.3: 16-bit sizes */
#else
typedef uint32_t usbd_size_t;
#endif
USBD_StatusTypeDef USBD_LL_Transmit(USBD_HandleTypeDef *pdev, uint8_t ep, uint8_t *buf, usbd_size_t size) { return st(HAL_PCD_EP_Transmit(pdev->pData, ep, buf, size)); }
USBD_StatusTypeDef USBD_LL_PrepareReceive(USBD_HandleTypeDef *pdev, uint8_t ep, uint8_t *buf, usbd_size_t size) { return st(HAL_PCD_EP_Receive(pdev->pData, ep, buf, size)); }
uint32_t USBD_LL_GetRxDataSize(USBD_HandleTypeDef *pdev, uint8_t ep) { return HAL_PCD_EP_GetRxCount(pdev->pData, ep); }
void USBD_LL_Delay(uint32_t ms) { HAL_Delay(ms); }

void *USBD_static_malloc(uint32_t size)
{
  static uint32_t mem[(sizeof(USBD_CDC_HandleTypeDef) / 4U) + 1U];
  (void)size;
  return mem;
}
void USBD_static_free(void *p) { (void)p; }
