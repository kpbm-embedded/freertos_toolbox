/* Message buffers carry whole variable-length messages (each stored with its length). A sender task posts
 * messages of 1..40 bytes; the receiver checks each one. A receive into a too-small buffer returns 0 and leaves
 * the message in place, so it can be read again with a bigger buffer. */
#include <string.h>
#include "FreeRTOS.h"
#include "message_buffer.h"
#include "task.h"
#include "board.h"

#define N_MSG 50
static MessageBufferHandle_t mb;

static size_t make_msg(uint32_t i, char *out)
{
  size_t len = 1 + (i * 7) % 40;
  for (size_t k = 0; k < len; k++) out[k] = (char)('a' + (i + k) % 26);
  return len;
}

static void sender(void *arg)
{
  char msg[40];
  for (uint32_t i = 0; i < N_MSG; i++) {
    size_t len = make_msg(i, msg);
    xMessageBufferSend(mb, msg, len, portMAX_DELAY);
  }
  vTaskDelete(NULL);
}

static void receiver(void *arg)
{
  char got[40], want[40], small[4];
  uint32_t ok = 0, small_refused = 0;
  for (uint32_t i = 0; i < N_MSG; i++) {
    size_t wlen = make_msg(i, want);
    if (wlen > sizeof small) {               /* try a too-small buffer first: must refuse, message stays */
      if (xMessageBufferReceive(mb, small, sizeof small, pdMS_TO_TICKS(500)) == 0) small_refused++;
    }
    size_t n = xMessageBufferReceive(mb, got, sizeof got, pdMS_TO_TICKS(500));
    if (n == wlen && memcmp(got, want, n) == 0) ok++;
  }
  log_printf("%lu of %d messages intact, %lu too-small reads refused\n", (unsigned long)ok, N_MSG,
             (unsigned long)small_refused);
  test_check(ok == N_MSG, "every message arrived whole with its own length");
  test_check(small_refused > 0, "receive into a too-small buffer returned 0");
  test_check(xMessageBufferIsEmpty(mb) == pdTRUE, "buffer empty at the end");
  test_check(xMessageBufferSpacesAvailable(mb) == 256, "all space free again");
  test_done();
}

int main(void)
{
  log_printf("qemu-message-buffer: variable-length messages\n");
  mb = xMessageBufferCreate(256);
  xTaskCreate(sender, "sender", 256, NULL, 1, NULL);
  xTaskCreate(receiver, "receiver", 512, NULL, 2, NULL);
  vTaskStartScheduler();
  return 1;
}
