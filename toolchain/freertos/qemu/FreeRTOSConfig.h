/* FreeRTOS configuration for the qemu mps2-an385 examples (ARM_CM3 port). An example overrides any value by
 * defining it in its own app_config.h (found on the include path first); everything here is a default. */
#pragma once
#include <stdint.h>
#if defined(__has_include)
#if __has_include("app_config.h")
#include "app_config.h"
#endif
#endif

#ifndef configCPU_CLOCK_HZ
#define configCPU_CLOCK_HZ                       25000000u
#endif
#ifndef configTICK_RATE_HZ
#define configTICK_RATE_HZ                       1000
#endif
#define configUSE_PREEMPTION                     1
#define configUSE_TIME_SLICING                   1
#define configMAX_PRIORITIES                     7
#define configMINIMAL_STACK_SIZE                 256
#define configMAX_TASK_NAME_LEN                  16
#define configTICK_TYPE_WIDTH_IN_BITS            TICK_TYPE_WIDTH_32_BITS
#define configIDLE_SHOULD_YIELD                  1
#define configUSE_MUTEXES                        1
#define configUSE_RECURSIVE_MUTEXES              1
#define configUSE_COUNTING_SEMAPHORES            1
#define configUSE_TASK_NOTIFICATIONS             1
#ifndef configTASK_NOTIFICATION_ARRAY_ENTRIES
#define configTASK_NOTIFICATION_ARRAY_ENTRIES    3
#endif
#define configQUEUE_REGISTRY_SIZE                8
#ifndef configUSE_IDLE_HOOK
#define configUSE_IDLE_HOOK                      0
#endif
#ifndef configUSE_TICK_HOOK
#define configUSE_TICK_HOOK                      0
#endif
#ifndef configUSE_TICKLESS_IDLE
#define configUSE_TICKLESS_IDLE                  0
#endif
#define configUSE_TIMERS                         1
#define configTIMER_TASK_PRIORITY                (configMAX_PRIORITIES - 1)
#define configTIMER_QUEUE_LENGTH                 10
#define configTIMER_TASK_STACK_DEPTH             512
#ifndef configSUPPORT_DYNAMIC_ALLOCATION
#define configSUPPORT_DYNAMIC_ALLOCATION         1
#endif
#ifndef configSUPPORT_STATIC_ALLOCATION
#define configSUPPORT_STATIC_ALLOCATION          0
#endif
#ifndef configKERNEL_PROVIDED_STATIC_MEMORY
#define configKERNEL_PROVIDED_STATIC_MEMORY      0
#endif
#ifndef configTOTAL_HEAP_SIZE
#define configTOTAL_HEAP_SIZE                    (64 * 1024)
#endif
#ifndef configCHECK_FOR_STACK_OVERFLOW
#define configCHECK_FOR_STACK_OVERFLOW           2
#endif
#ifndef configUSE_MALLOC_FAILED_HOOK
#define configUSE_MALLOC_FAILED_HOOK             configSUPPORT_DYNAMIC_ALLOCATION
#endif
#define configUSE_TRACE_FACILITY                 1
#ifndef configUSE_STATS_FORMATTING_FUNCTIONS
#define configUSE_STATS_FORMATTING_FUNCTIONS     configSUPPORT_DYNAMIC_ALLOCATION
#endif
#ifndef configGENERATE_RUN_TIME_STATS
#define configGENERATE_RUN_TIME_STATS            0
#endif
#define configRECORD_STACK_HIGH_ADDRESS          1

/* Cortex-M3 interrupt priorities as raw 8-bit NVIC values: they work with any number of implemented bits.
 * ISRs that call FromISR APIs must use a numerically higher (= lower urgency) value, e.g. BOARD_IRQ_PRIO_LOW (0xE0). */
#define configKERNEL_INTERRUPT_PRIORITY          0xFFu
#define configMAX_SYSCALL_INTERRUPT_PRIORITY     0xA0u

void board_assert_failed(const char *file, int line);
#define configASSERT(x) do { if (!(x)) board_assert_failed(__FILE__, __LINE__); } while (0)

#define INCLUDE_vTaskPrioritySet                 1
#define INCLUDE_uxTaskPriorityGet                1
#define INCLUDE_vTaskDelete                      1
#define INCLUDE_vTaskSuspend                     1
#define INCLUDE_vTaskDelay                       1
#define INCLUDE_xTaskDelayUntil                  1
#define INCLUDE_xTaskGetSchedulerState           1
#define INCLUDE_xTaskGetCurrentTaskHandle        1
#define INCLUDE_uxTaskGetStackHighWaterMark      1
#define INCLUDE_xTimerPendFunctionCall           1
#define INCLUDE_xTaskGetHandle                   1
#define INCLUDE_eTaskGetState                    1
#define INCLUDE_xTaskGetIdleTaskHandle           1
#define INCLUDE_xSemaphoreGetMutexHolder         1
#define INCLUDE_xTaskAbortDelay                  1
