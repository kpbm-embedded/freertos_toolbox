/* Descriptors for a CDC-ACM virtual COM port. VID/PID 0483:5740 is ST's published VCP pair - use your own for products.
 * The serial number string is built from the chip's 96-bit unique ID (UID_BASE). */
#include "usbd_core.h"
#include "usbd_desc.h"

#define USBD_VID 0x0483
#define USBD_PID 0x5740

static uint8_t str_buf[USBD_MAX_STR_DESC_SIZ];

static uint8_t dev_desc[USB_LEN_DEV_DESC] = {
  0x12, USB_DESC_TYPE_DEVICE, 0x00, 0x02,             /* bcdUSB 2.00 */
  0x02, 0x00, 0x00, USB_MAX_EP0_SIZE,                 /* class: communications, EP0 64 */
  LOBYTE(USBD_VID), HIBYTE(USBD_VID), LOBYTE(USBD_PID), HIBYTE(USBD_PID),
  0x00, 0x02, USBD_IDX_MFC_STR, USBD_IDX_PRODUCT_STR, USBD_IDX_SERIAL_STR, USBD_MAX_NUM_CONFIGURATION };
static uint8_t langid[USB_LEN_LANGID_STR_DESC] = { USB_LEN_LANGID_STR_DESC, USB_DESC_TYPE_STRING, 0x09, 0x04 };

static uint8_t *dev(USBD_SpeedTypeDef s, uint16_t *l) { (void)s; *l = sizeof dev_desc; return dev_desc; }
static uint8_t *lang(USBD_SpeedTypeDef s, uint16_t *l) { (void)s; *l = sizeof langid; return langid; }
static uint8_t *str(const char *s, uint16_t *l) { USBD_GetString((uint8_t *)s, str_buf, l); return str_buf; }
static uint8_t *mfc(USBD_SpeedTypeDef s, uint16_t *l) { (void)s; return str("mcu examples", l); }
static uint8_t *prod(USBD_SpeedTypeDef s, uint16_t *l) { (void)s; return str("FreeRTOS CDC example", l); }
static uint8_t *cfg(USBD_SpeedTypeDef s, uint16_t *l) { (void)s; return str("CDC config", l); }
static uint8_t *itf(USBD_SpeedTypeDef s, uint16_t *l) { (void)s; return str("CDC interface", l); }
static uint8_t *serial(USBD_SpeedTypeDef s, uint16_t *l)
{
  (void)s;
  char hex[13];
  uint32_t id = *(volatile uint32_t *)UID_BASE + *(volatile uint32_t *)(UID_BASE + 4U);
  uint32_t id2 = *(volatile uint32_t *)(UID_BASE + 8U);
  for (int i = 0; i < 6; i++) hex[i] = "0123456789ABCDEF"[(id >> (28 - 4 * i)) & 0xf];
  for (int i = 0; i < 6; i++) hex[6 + i] = "0123456789ABCDEF"[(id2 >> (28 - 4 * i)) & 0xf];
  hex[12] = 0;
  return str(hex, l);
}

USBD_DescriptorsTypeDef CDC_Desc = { dev, lang, mfc, prod, serial, cfg, itf };
