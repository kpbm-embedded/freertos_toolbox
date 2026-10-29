#include <stdarg.h>
#include <stdio.h>
#include "board.h"
#include "rtos_glue.h"
#include "platform.h"

void platform_init(void) { board_init(); rtos_log_init(); }
void platform_log(const char *fmt, ...)
{
  char b[160]; va_list ap; va_start(ap, fmt); vsnprintf(b, sizeof b, fmt, ap); va_end(ap);
  rtos_log("%s", b);
}
void platform_led(int on) { board_led_set(on); }
void platform_finish(int ok) { platform_log("self-check %s", ok ? "ok" : "FAILED"); }
