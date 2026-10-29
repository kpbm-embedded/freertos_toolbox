/* A timer interrupt (2 kHz) writes one byte of a counting pattern per tick into a stream buffer, like a UART RX
 * ISR would. The reader task wakes when 16 bytes are waiting (trigger level) and checks that no byte was lost,
 * duplicated or reordered. Stream buffers have one writer and one reader: no locking needed. */
#include "FreeRTOS.h"
#include "stream_buffer.h"
#include "task.h"
#include "board.h"

#define TOTAL 600
static StreamBufferHandle_t sb;
static volatile uint32_t sent, dropped;

void board_timer1_isr(void)
{
  if (sent >= TOTAL) return;
  uint8_t byte = (uint8_t)(sent & 0xFF);
  BaseType_t woken = pdFALSE;
  if (xStreamBufferSendFromISR(sb, &byte, 1, &woken) == 1) sent++; else dropped++;
  portYIELD_FROM_ISR(woken);
}

static void reader(void *arg)
{
  uint8_t buf[64];
  uint32_t got = 0, wakeups = 0, bad = 0;
  board_timer1_start(2000);
  while (got < TOTAL) {
    size_t n = xStreamBufferReceive(sb, buf, sizeof buf, pdMS_TO_TICKS(500));
    if (n == 0) break;
    wakeups++;
    for (size_t i = 0; i < n; i++, got++)
      if (buf[i] != (uint8_t)(got & 0xFF)) bad++;
  }
  board_timer1_stop();
  log_printf("received %lu bytes in %lu wakeups, %lu bad, %lu dropped by the ISR\n", (unsigned long)got,
             (unsigned long)wakeups, (unsigned long)bad, (unsigned long)dropped);
  test_check(got == TOTAL, "all bytes received");
  test_check(bad == 0, "bytes in order, none lost or duplicated");
  test_check(dropped == 0, "buffer never overflowed");
  test_check(wakeups < got / 4, "trigger level batched the bytes (far fewer wakeups than bytes)");
  test_done();
}

int main(void)
{
  log_printf("qemu-stream-buffer: timer ISR -> stream buffer -> task\n");
  sb = xStreamBufferCreate(128, 16);
  xTaskCreate(reader, "reader", 512, NULL, 2, NULL);
  vTaskStartScheduler();
  return 1;
}
