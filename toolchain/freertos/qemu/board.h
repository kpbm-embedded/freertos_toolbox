/* qemu mps2-an385 board layer for the FreeRTOS examples: CMSDK UART0 console, CMSDK timers, semihosting exit,
 * and a tiny self-test protocol. Every example ends with "RESULT: PASS" (exit 0) or "RESULT: FAIL" (exit 1). */
#pragma once
#include <stdint.h>

#define BOARD_CPU_HZ        25000000u   /* mps2-an385 system clock as modelled by qemu */
#define BOARD_IRQ_UART0_RX  0
#define BOARD_IRQ_TIMER0    8
#define BOARD_IRQ_TIMER1    9
#define BOARD_IRQ_SOFT      31          /* unused line, raised only by software (board_trigger_irq) */
#define BOARD_IRQ_PRIO_LOW  0xE0u       /* raw NVIC priority: below configMAX_SYSCALL_INTERRUPT_PRIORITY, may call FromISR */

void board_init(void);
void board_putc(char c);
void board_puts(const char *s);
int  board_getc(void);                         /* -1 when no byte is waiting */
void log_printf(const char *fmt, ...) __attribute__((format(printf, 1, 2)));

void board_irq_enable(int irq, uint8_t prio);
void board_irq_disable(int irq);
void board_trigger_irq(int irq);               /* software-triggered interrupt (NVIC STIR) */
void board_timer1_start(uint32_t hz);          /* periodic TIMER1 interrupt -> board_timer1_isr() */
void board_timer1_stop(void);
void board_timer1_isr(void);                   /* weak; override in the app */
void board_soft_isr(void);                     /* weak; IRQ 31 handler, override in the app */
void board_uart0_rx_isr(void);                 /* weak; UART0 RX handler */
uint32_t board_cycles(void);                   /* free-running 25 MHz counter (TIMER0, counts up) */

void board_exit(int code) __attribute__((noreturn));   /* semihosting SYS_EXIT: qemu exits with 0 or 1 */

/* self-test protocol */
void test_check(int ok, const char *what);
int  test_failures(void);
void test_done(void) __attribute__((noreturn)); /* prints RESULT and exits */
