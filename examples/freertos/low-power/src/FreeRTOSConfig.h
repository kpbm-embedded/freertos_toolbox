/* low-power: the shared config (toolchain/freertos/common/FreeRTOSConfig.h) plus tickless idle. This file shadows it because
 * CMakeLists.txt puts src/ first on the include path; #include_next then pulls in the shared one. */
#pragma once
#include_next "FreeRTOSConfig.h"

#undef  configUSE_TICKLESS_IDLE
#define configUSE_TICKLESS_IDLE                  1      /* port's default vPortSuppressTicksAndSleep: SysTick reprogrammed, WFI */
#define configEXPECTED_IDLE_TIME_BEFORE_SLEEP    5      /* ticks; shorter idle periods keep the tick running */

/* Hooks around the WFI inside vPortSuppressTicksAndSleep (interrupts are masked there): count sleeps for the statistics. */
#ifndef __ASSEMBLER__
#include <stdint.h>
void lp_pre_sleep(uint32_t expected_ticks);
void lp_post_sleep(uint32_t expected_ticks);
#endif
#define configPRE_SLEEP_PROCESSING(x)            lp_pre_sleep(x)
#define configPOST_SLEEP_PROCESSING(x)           lp_post_sleep(x)
