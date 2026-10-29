#define configUSE_TICKLESS_IDLE               1
#define configEXPECTED_IDLE_TIME_BEFORE_SLEEP 5
extern volatile unsigned long app_sleeps;
#define configPRE_SLEEP_PROCESSING(x)  do { app_sleeps++; /* stop clocks, enter a low-power mode here */ } while (0)
#define configPOST_SLEEP_PROCESSING(x) do { /* restore clocks here */ } while (0)
