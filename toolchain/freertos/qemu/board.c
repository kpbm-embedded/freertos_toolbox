/* qemu mps2-an385 board layer: see board.h. Register maps: Arm CMSDK APB UART / timer (AN385 memory map). */
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include "board.h"

#define REG(a) (*(volatile uint32_t *)(a))
#define UART0      0x40004000u
#define UART_DATA  0x00u
#define UART_STATE 0x04u
#define UART_CTRL  0x08u
#define UART_INT   0x0Cu
#define UART_BAUD  0x10u
#define TIMER0     0x40000000u
#define TIMER1     0x40001000u
#define T_CTRL 0x0u
#define T_VALUE 0x4u
#define T_RELOAD 0x8u
#define NVIC_ISER 0xE000E100u
#define NVIC_ICER 0xE000E180u
#define NVIC_IPR  0xE000E400u
#define NVIC_STIR 0xE000EF00u

static int failures, checks;

void board_init(void)
{
  REG(UART0 + UART_BAUD) = 16;
  REG(UART0 + UART_CTRL) = 0x3;                     /* TX + RX enable */
  REG(TIMER0 + T_RELOAD) = 0xFFFFFFFFu;             /* free-running cycle counter */
  REG(TIMER0 + T_VALUE) = 0xFFFFFFFFu;
  REG(TIMER0 + T_CTRL) = 0x1;
}

void board_putc(char c)
{
  while (REG(UART0 + UART_STATE) & 1u) { }
  REG(UART0 + UART_DATA) = (uint8_t)c;
}

void board_puts(const char *s) { while (*s) board_putc(*s++); }

int board_getc(void)
{
  if (REG(UART0 + UART_STATE) & 2u) return (int)(REG(UART0 + UART_DATA) & 0xFFu);
  return -1;
}

/* Whole lines are written with interrupts masked, so lines from different tasks never mix. */
void log_printf(const char *fmt, ...)
{
  char buf[192];
  va_list ap;
  va_start(ap, fmt);
  vsnprintf(buf, sizeof buf, fmt, ap);
  va_end(ap);
  uint32_t primask;
  __asm volatile("mrs %0, primask\n cpsid i" : "=r"(primask)::"memory");
  board_puts(buf);
  __asm volatile("msr primask, %0" ::"r"(primask) : "memory");
}

/* newlib: printf() and friends end here */
int _write(int fd, const char *p, int n)
{
  (void)fd;
  for (int i = 0; i < n; i++) board_putc(p[i]);
  return n;
}

/* the other newlib system calls: no files, no processes */
#include <sys/stat.h>
int _read(int fd, char *p, int n) { (void)fd; (void)p; (void)n; return 0; }
int _close(int fd) { (void)fd; return -1; }
int _lseek(int fd, int off, int whence) { (void)fd; (void)off; (void)whence; return 0; }
int _fstat(int fd, struct stat *st) { (void)fd; st->st_mode = S_IFCHR; return 0; }
int _isatty(int fd) { (void)fd; return 1; }
int _getpid(void) { return 1; }
int _kill(int pid, int sig) { (void)pid; (void)sig; return -1; }
void _exit(int code) { board_exit(code); }
void *_sbrk(int incr)
{
  extern char end;                    /* from the linker script; newlib's malloc only (FreeRTOS uses its own heap) */
  static char *brk = &end;
  char *prev = brk;
  brk += incr;
  return prev;
}

void board_irq_enable(int irq, uint8_t prio)
{
  ((volatile uint8_t *)NVIC_IPR)[irq] = prio;
  REG(NVIC_ISER + 4u * ((unsigned)irq / 32u)) = 1u << ((unsigned)irq % 32u);
}

void board_irq_disable(int irq) { REG(NVIC_ICER + 4u * ((unsigned)irq / 32u)) = 1u << ((unsigned)irq % 32u); }

void board_trigger_irq(int irq) { REG(NVIC_STIR) = (uint32_t)irq; }

void board_timer1_start(uint32_t hz)
{
  REG(TIMER1 + T_CTRL) = 0;
  REG(TIMER1 + T_RELOAD) = BOARD_CPU_HZ / hz - 1u;
  REG(TIMER1 + T_VALUE) = BOARD_CPU_HZ / hz - 1u;
  board_irq_enable(BOARD_IRQ_TIMER1, BOARD_IRQ_PRIO_LOW);
  REG(TIMER1 + T_CTRL) = 0x9;                       /* enable + interrupt enable */
}

void board_timer1_stop(void)
{
  REG(TIMER1 + T_CTRL) = 0;
  board_irq_disable(BOARD_IRQ_TIMER1);
}

uint32_t board_cycles(void) { return 0xFFFFFFFFu - REG(TIMER0 + T_VALUE); }

__attribute__((weak)) void board_timer1_isr(void) { }
__attribute__((weak)) void board_soft_isr(void) { }
__attribute__((weak)) void board_uart0_rx_isr(void) { }

void board_exit(int code)
{
  /* ARM semihosting SYS_EXIT (0x18): ADP_Stopped_ApplicationExit -> qemu exit 0, anything else -> exit 1 */
  register uint32_t r0 __asm("r0") = 0x18;
  register uint32_t r1 __asm("r1") = code == 0 ? 0x20026u : 0x20023u;
  __asm volatile("bkpt 0xAB" : : "r"(r0), "r"(r1) : "memory");
  for (;;) { }
}

void test_check(int ok, const char *what)
{
  checks++;
  if (!ok) failures++;
  log_printf("CHECK %s: %s\n", ok ? "ok  " : "FAIL", what);
}

int test_failures(void) { return failures; }

void test_done(void)
{
  if (failures == 0 && checks > 0) {
    log_printf("RESULT: PASS (%d checks)\n", checks);
    board_exit(0);
  }
  log_printf("RESULT: FAIL (%d of %d checks failed)\n", failures, checks);
  board_exit(1);
}
