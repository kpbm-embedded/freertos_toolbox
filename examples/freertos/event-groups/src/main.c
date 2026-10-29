/* Three workers finish at different rates and set their bit; the collector waits for ALL bits (and clears them on exit),
 * so it prints one line per completed round. */
#include "board.h"
#include "FreeRTOS.h"
#include "task.h"
#include "event_groups.h"
#include "rtos_glue.h"

#define BIT_A (1U << 0)
#define BIT_B (1U << 1)
#define BIT_C (1U << 2)
static EventGroupHandle_t eg;

static void worker(void *arg)
{
  uint32_t bit = (uint32_t)(uintptr_t)arg;
  uint32_t period = bit == BIT_A ? 300 : (bit == BIT_B ? 500 : 800);
  for (;;) {
    vTaskDelay(pdMS_TO_TICKS(period));
    xEventGroupSetBits(eg, bit);
  }
}
static void collector(void *arg)
{
  (void)arg;
  for (unsigned round = 0;; round++) {
    TickType_t t0 = xTaskGetTickCount();
    EventBits_t b = xEventGroupWaitBits(eg, BIT_A | BIT_B | BIT_C, pdTRUE /* clear on exit */, pdTRUE /* wait all */, pdMS_TO_TICKS(5000));
    rtos_log("round %u: bits=0x%x after %lu ms", round, (unsigned)b, (unsigned long)(xTaskGetTickCount() - t0));
    board_led_toggle();
  }
}

int main(void)
{
  board_init();
  rtos_log_init();
  eg = xEventGroupCreate();
  xTaskCreate(worker, "wA", 256, (void *)(uintptr_t)BIT_A, 1, NULL);
  xTaskCreate(worker, "wB", 256, (void *)(uintptr_t)BIT_B, 1, NULL);
  xTaskCreate(worker, "wC", 256, (void *)(uintptr_t)BIT_C, 1, NULL);
  xTaskCreate(collector, "collector", 384, NULL, 2, NULL);
  vTaskStartScheduler();
  for (;;) {}
}
