/* C shims for the FreeRTOS calls the Rust/Zig task uses: many kernel "functions" are macros (xQueueSend,
 * pdMS_TO_TICKS), which no foreign language can call, so each gets a real C function here. */
#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"
#include "board.h"

int rtos_queue_send(void *q, const void *item, uint32_t wait_ms) { return xQueueSend((QueueHandle_t)q, item, pdMS_TO_TICKS(wait_ms)) == pdPASS; }
int rtos_queue_receive(void *q, void *item, uint32_t wait_ms) { return xQueueReceive((QueueHandle_t)q, item, pdMS_TO_TICKS(wait_ms)) == pdPASS; }
void rtos_delay_ms(uint32_t ms) { vTaskDelay(pdMS_TO_TICKS(ms)); }
uint32_t rtos_tick_ms(void) { return (uint32_t)(xTaskGetTickCount() * portTICK_PERIOD_MS); }
void rtos_log(const char *s) { log_printf("%s", s); }
void rtos_suspend_self(void) { vTaskSuspend(NULL); }
