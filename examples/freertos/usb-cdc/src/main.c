/* usb-cdc: USB CDC-ACM virtual COM port (ST Cube USB Device library) under FreeRTOS. The CDC receive callback runs in the USB
 * interrupt: it only pushes the packet into a stream buffer (xStreamBufferSendFromISR) and re-arms reception. An "echo" task
 * drains the stream buffer and sends the bytes back in 64-byte packets; a "stats" task logs the byte count on USART1. */
#include "board.h"
#include "FreeRTOS.h"
#include "task.h"
#include "stream_buffer.h"
#include "rtos_glue.h"
#include "usbd_core.h"
#include "usbd_cdc.h"
#include "usbd_desc.h"

static USBD_HandleTypeDef hUsb;
static uint8_t rxbuf[CDC_DATA_FS_MAX_PACKET_SIZE], txbuf[CDC_DATA_FS_MAX_PACKET_SIZE];
static StreamBufferHandle_t rx_sb;
static volatile uint32_t total_rx, total_tx, dropped;

static int8_t cdc_init(void) { USBD_CDC_SetTxBuffer(&hUsb, txbuf, 0); USBD_CDC_SetRxBuffer(&hUsb, rxbuf); return USBD_OK; }
static int8_t cdc_deinit(void) { return USBD_OK; }
static int8_t cdc_control(uint8_t cmd, uint8_t *pbuf, uint16_t len)
{
  static uint8_t line[7] = { 0x00, 0xC2, 0x01, 0x00, 0x00, 0x00, 0x08 };   /* 115200 8N1 reported to the host */
  (void)len;
  if (cmd == CDC_SET_LINE_CODING) memcpy(line, pbuf, 7);
  else if (cmd == CDC_GET_LINE_CODING) memcpy(pbuf, line, 7);
  return USBD_OK;
}
static int8_t cdc_receive(uint8_t *buf, uint32_t *len)          /* USB ISR context */
{
  BaseType_t woken = pdFALSE;
  size_t put = xStreamBufferSendFromISR(rx_sb, buf, *len, &woken);
  total_rx += *len;
  dropped += *len - put;
  USBD_CDC_SetRxBuffer(&hUsb, rxbuf);
  USBD_CDC_ReceivePacket(&hUsb);
  portYIELD_FROM_ISR(woken);
  return USBD_OK;
}
static USBD_CDC_ItfTypeDef fops = { cdc_init, cdc_deinit, cdc_control, cdc_receive };   /* TransmitCplt (newer libs) left NULL */

static int cdc_tx_busy(void)
{
  USBD_CDC_HandleTypeDef *c = (USBD_CDC_HandleTypeDef *)hUsb.pClassData;
  return c == NULL || c->TxState != 0U;
}

static void echo_task(void *arg)
{
  (void)arg;
  uint8_t chunk[CDC_DATA_FS_MAX_PACKET_SIZE];
  for (;;) {
    size_t n = xStreamBufferReceive(rx_sb, chunk, sizeof chunk, portMAX_DELAY);
    for (int tries = 0; cdc_tx_busy() && tries < 50; tries++) vTaskDelay(1);   /* previous IN packet still pending? */
    if (cdc_tx_busy()) continue;                                                /* host not reading: drop */
    memcpy(txbuf, chunk, n);
    taskENTER_CRITICAL();                     /* masks the USB IRQ (priority 6) while the endpoint is armed */
    USBD_CDC_SetTxBuffer(&hUsb, txbuf, (uint32_t)n);
    USBD_CDC_TransmitPacket(&hUsb);
    taskEXIT_CRITICAL();
    total_tx += n;
    board_led_toggle();
  }
}

static void stats_task(void *arg)
{
  (void)arg;
  uint32_t seen = 0;
  for (;;) {
    vTaskDelay(pdMS_TO_TICKS(1000));
    if (total_rx != seen) {
      seen = total_rx;
      rtos_log("usb: rx %lu B, echoed %lu B, dropped %lu B", (unsigned long)seen, (unsigned long)total_tx, (unsigned long)dropped);
    }
  }
}

int main(void)
{
  board_init();
  rtos_log_init();
  rx_sb = xStreamBufferCreate(256, 1);
  USBD_Init(&hUsb, &CDC_Desc, 0);
  USBD_RegisterClass(&hUsb, &USBD_CDC);
  USBD_CDC_RegisterInterface(&hUsb, &fops);
  USBD_Start(&hUsb);
  rtos_print("\r\nfreertos usb-cdc on %s: open the new /dev/ttyACM* and type\r\n", BOARD_NAME);
  xTaskCreate(echo_task, "echo", 256, NULL, 3, NULL);
  xTaskCreate(stats_task, "stats", 256, NULL, 1, NULL);
  vTaskStartScheduler();
  for (;;) {}
}
