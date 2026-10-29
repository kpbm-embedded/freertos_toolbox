/* A FreeRTOS task sums the same 100 samples as qemu-rust-task; test.sh checks the value against src/stats.bend run
 * on the host with Bend 2 (32_31_bend). Bend has no MCU target: it only provides the host-side reference. */
#include "FreeRTOS.h"
#include "task.h"
#include "board.h"

static void worker(void *arg)
{
  int32_t sum = 0;
  for (int32_t i = 0; i < 100; i++) sum += (i * 37) % 101 - 50;
  log_printf("sum = %ld\n", (long)sum);
  test_check(sum == -14, "sum of the 100 samples");
  test_done();
}

int main(void)
{
  xTaskCreate(worker, "worker", 512, NULL, 1, NULL);
  vTaskStartScheduler();
  return 1;
}
