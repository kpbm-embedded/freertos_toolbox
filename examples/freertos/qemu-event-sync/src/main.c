/* Event groups three ways:
 *  - barrier: three tasks call xEventGroupSync() each round; none may start round n+1 before all finished round n;
 *  - wait-all / wait-any with a timeout;
 *  - xEventGroupSetBitsFromISR() from a software-triggered interrupt (deferred to the timer service task). */
#include "FreeRTOS.h"
#include "event_groups.h"
#include "task.h"
#include "board.h"

#define BIT_A (1u << 0)
#define BIT_B (1u << 1)
#define BIT_C (1u << 2)
#define ALL_SYNC (BIT_A | BIT_B | BIT_C)
#define BIT_ISR (1u << 8)
#define ROUNDS 5

static EventGroupHandle_t barrier, flags;
static volatile uint32_t round_of[3];
static volatile int order_ok = 1;

void board_soft_isr(void)
{
  BaseType_t woken = pdFALSE;
  xEventGroupSetBitsFromISR(flags, BIT_ISR, &woken);
  portYIELD_FROM_ISR(woken);
}

static void party(void *arg)
{
  uint32_t me = (uint32_t)arg, bit = 1u << me;
  for (uint32_t r = 1; r <= ROUNDS; r++) {
    vTaskDelay(pdMS_TO_TICKS(2 + 5 * me));             /* different amounts of work */
    round_of[me] = r;
    xEventGroupSync(barrier, bit, ALL_SYNC, portMAX_DELAY);
    for (int o = 0; o < 3; o++)                          /* after the barrier everyone has finished round r */
      if (round_of[o] < r) order_ok = 0;
  }
  xEventGroupSetBits(flags, bit);
  vTaskDelete(NULL);
}

static void checker(void *arg)
{
  EventBits_t b = xEventGroupWaitBits(flags, BIT_A | BIT_B | BIT_C, pdFALSE, pdTRUE, pdMS_TO_TICKS(2000));
  test_check((b & ALL_SYNC) == ALL_SYNC, "wait-all: all three parties finished");
  test_check(order_ok, "barrier: nobody ran ahead by a round");

  b = xEventGroupWaitBits(flags, 1u << 5, pdTRUE, pdFALSE, pdMS_TO_TICKS(30));
  test_check((b & (1u << 5)) == 0, "wait-any on an unset bit times out");

  board_irq_enable(BOARD_IRQ_SOFT, BOARD_IRQ_PRIO_LOW);
  board_trigger_irq(BOARD_IRQ_SOFT);
  b = xEventGroupWaitBits(flags, BIT_ISR, pdTRUE, pdFALSE, pdMS_TO_TICKS(100));
  test_check(b & BIT_ISR, "bit set from an ISR woke the waiting task");
  test_check((xEventGroupGetBits(flags) & BIT_ISR) == 0, "clear-on-exit cleared the ISR bit");
  test_done();
}

int main(void)
{
  log_printf("qemu-event-sync: barrier, wait-any/all, ISR bits\n");
  barrier = xEventGroupCreate();
  flags = xEventGroupCreate();
  for (uint32_t i = 0; i < 3; i++) xTaskCreate(party, "party", 256, (void *)i, 2, NULL);
  xTaskCreate(checker, "checker", 512, NULL, 1, NULL);
  vTaskStartScheduler();
  return 1;
}
