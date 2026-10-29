/* Tickless idle with the ARM_CM3 port's default vPortSuppressTicksAndSleep (SysTick reprogrammed, WFI). */
#define configUSE_TICKLESS_IDLE               1
#define configEXPECTED_IDLE_TIME_BEFORE_SLEEP 5
extern volatile unsigned long app_sleeps, app_wakeups;
#define configPRE_SLEEP_PROCESSING(x)  do { app_sleeps++; } while (0)    /* e.g. gate clocks, enter STOP */
#define configPOST_SLEEP_PROCESSING(x) do { app_wakeups++; } while (0)   /* e.g. restore clocks */
