/* Static allocation only: no FreeRTOS heap is linked (freertos_qemu(... HEAP none)); test.sh checks the ELF. */
#define configSUPPORT_DYNAMIC_ALLOCATION   0
#define configSUPPORT_STATIC_ALLOCATION    1
#define configKERNEL_PROVIDED_STATIC_MEMORY 0   /* we provide the idle and timer task memory below */
