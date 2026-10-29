/* Task notifications are the lightest ISR/task-to-task signal (no kernel object). Five uses:
 *  1. binary semaphore from an ISR: vTaskNotifyGiveFromISR + ulTaskNotifyTake(pdTRUE)
 *  2. counting semaphore: several gives, ulTaskNotifyTake(pdFALSE) takes one at a time
 *  3. event bits: xTaskNotify(eSetBits) + xTaskNotifyWait()
 *  4. mailbox: eSetValueWithoutOverwrite fails while a value is pending, eSetValueWithOverwrite replaces it
 *  5. indexed notifications (configTASK_NOTIFICATION_ARRAY_ENTRIES = 3): index 1 is independent of index 0 */
#include "FreeRTOS.h"
#include "task.h"
#include "board.h"

static TaskHandle_t target;

void board_soft_isr(void)
{
  BaseType_t woken = pdFALSE;
  vTaskNotifyGiveFromISR(target, &woken);
  portYIELD_FROM_ISR(woken);
}

static void target_task(void *arg)
{
  uint32_t v;
  /* 1: ISR give */
  v = ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(100));
  test_check(v == 1, "1 binary: notified from the ISR");
  /* 2: counting (the checker gives 3 times) */
  xTaskNotifyGive(xTaskGetHandle("checker"));
  uint32_t takes = 0;
  while (ulTaskNotifyTake(pdFALSE, pdMS_TO_TICKS(20)) > 0) takes++;
  test_check(takes == 3, "2 counting: three gives, three takes");
  /* 3: event bits */
  uint32_t bits = 0;
  xTaskNotifyWait(0, 0xFFFFFFFFu, &bits, pdMS_TO_TICKS(100));
  test_check(bits == 0x5, "3 bits: 0x1 | 0x4 collected in one wait");
  /* 4: mailbox */
  xTaskNotifyWait(0, 0xFFFFFFFFu, &v, pdMS_TO_TICKS(100));
  test_check(v == 222, "4 mailbox: overwrite replaced the unread 111");
  /* 5: index 1 */
  v = ulTaskNotifyTakeIndexed(1, pdTRUE, pdMS_TO_TICKS(100));
  test_check(v == 1, "5 indexed: index 1 notified");
  test_check(ulTaskNotifyTakeIndexed(0, pdTRUE, 0) == 0, "5 indexed: index 0 untouched");
  test_done();
}

static void checker(void *arg)
{
  vTaskDelay(1);
  board_irq_enable(BOARD_IRQ_SOFT, BOARD_IRQ_PRIO_LOW);
  board_trigger_irq(BOARD_IRQ_SOFT);                         /* 1 */
  ulTaskNotifyTake(pdTRUE, portMAX_DELAY);                  /* target is ready for 2 */
  xTaskNotifyGive(target); xTaskNotifyGive(target); xTaskNotifyGive(target);
  vTaskDelay(pdMS_TO_TICKS(50));
  vTaskSuspendAll();                                        /* 3: both bits land before target runs */
  xTaskNotify(target, 0x1, eSetBits);
  xTaskNotify(target, 0x4, eSetBits);
  xTaskResumeAll();
  vTaskDelay(pdMS_TO_TICKS(10));
  vTaskSuspend(target);                                     /* 4: queue two values while target cannot read */
  xTaskNotify(target, 111, eSetValueWithoutOverwrite);
  BaseType_t r = xTaskNotify(target, 999, eSetValueWithoutOverwrite);
  test_check(r == pdFAIL, "4 mailbox: no-overwrite refused while a value is pending");
  xTaskNotify(target, 222, eSetValueWithOverwrite);
  vTaskResume(target);
  vTaskDelay(pdMS_TO_TICKS(10));
  xTaskNotifyGiveIndexed(target, 1);                        /* 5 */
  vTaskDelay(portMAX_DELAY);
}

int main(void)
{
  log_printf("qemu-task-notify: five uses of task notifications\n");
  xTaskCreate(target_task, "target", 512, NULL, 2, &target);
  xTaskCreate(checker, "checker", 512, NULL, 1, NULL);
  vTaskStartScheduler();
  return 1;
}
