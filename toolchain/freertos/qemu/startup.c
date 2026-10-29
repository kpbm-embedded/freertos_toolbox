/* Startup for qemu mps2-an385 (Cortex-M3): vector table, .data/.bss init, fault reporting.
 * SVC/PendSV/SysTick go to the FreeRTOS ARM_CM3 port; SysTick is counted first (board_systick_count). */
#include <stdint.h>
#include "board.h"

extern uint32_t _estack, _sidata, _sdata, _edata, _sbss, _ebss;
extern int main(void);
extern void __libc_init_array(void);
extern void vPortSVCHandler(void);
extern void xPortPendSVHandler(void);
extern void xPortSysTickHandler(void);

volatile uint32_t board_systick_count;   /* SysTick interrupts taken (tickless idle shows fewer than ticks) */

void Reset_Handler(void);
static void Fault_Handler(void);
static void Default_Handler(void) { log_printf("unexpected interrupt\n"); board_exit(1); }
static void SysTick_Handler(void) { board_systick_count++; xPortSysTickHandler(); }
static void TIMER1_Handler(void);
static void SOFT_Handler(void) { board_soft_isr(); }
static void UART0RX_Handler(void) { board_uart0_rx_isr(); }

__attribute__((section(".isr_vector"), used)) static void (*const vectors[16 + 32])(void) = {
  (void (*)(void))&_estack, Reset_Handler, Default_Handler, Fault_Handler, Fault_Handler, Fault_Handler, Fault_Handler,
  0, 0, 0, 0, vPortSVCHandler, Default_Handler, 0, xPortPendSVHandler, SysTick_Handler,
  [16 + BOARD_IRQ_UART0_RX] = UART0RX_Handler,
  [16 + 1] = Default_Handler, [16 + 2] = Default_Handler, [16 + 3] = Default_Handler,
  [16 + 4] = Default_Handler, [16 + 5] = Default_Handler, [16 + 6] = Default_Handler, [16 + 7] = Default_Handler,
  [16 + BOARD_IRQ_TIMER0] = Default_Handler,
  [16 + BOARD_IRQ_TIMER1] = TIMER1_Handler,
  [16 + 10] = Default_Handler, [16 + 11] = Default_Handler, [16 + 12] = Default_Handler, [16 + 13] = Default_Handler,
  [16 + 14] = Default_Handler, [16 + 15] = Default_Handler, [16 + 16] = Default_Handler, [16 + 17] = Default_Handler,
  [16 + 18] = Default_Handler, [16 + 19] = Default_Handler, [16 + 20] = Default_Handler, [16 + 21] = Default_Handler,
  [16 + 22] = Default_Handler, [16 + 23] = Default_Handler, [16 + 24] = Default_Handler, [16 + 25] = Default_Handler,
  [16 + 26] = Default_Handler, [16 + 27] = Default_Handler, [16 + 28] = Default_Handler, [16 + 29] = Default_Handler,
  [16 + 30] = Default_Handler,
  [16 + BOARD_IRQ_SOFT] = SOFT_Handler,
};

void Reset_Handler(void)
{
  uint32_t *src = &_sidata, *dst = &_sdata;
  while (dst < &_edata) *dst++ = *src++;
  for (dst = &_sbss; dst < &_ebss; ) *dst++ = 0;
  *(volatile uint32_t *)0xE000ED08u = (uint32_t)vectors;   /* VTOR */
  __libc_init_array();
  board_init();
  board_exit(main());
}

static void Fault_Handler(void)
{
  volatile uint32_t *scb = (volatile uint32_t *)0xE000ED00u;
  log_printf("FAULT: HFSR=%08lx CFSR=%08lx MMFAR=%08lx BFAR=%08lx\n",
             (unsigned long)scb[0x2C / 4], (unsigned long)scb[0x28 / 4], (unsigned long)scb[0x34 / 4], (unsigned long)scb[0x38 / 4]);
  log_printf("RESULT: FAIL (fault)\n");
  board_exit(1);
}

static void TIMER1_Handler(void)
{
  *(volatile uint32_t *)0x4000100Cu = 1;   /* TIMER1 INTCLEAR */
  board_timer1_isr();
}
