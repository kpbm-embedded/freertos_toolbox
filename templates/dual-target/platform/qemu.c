#include <stdarg.h>
#include <stdio.h>
#include "board.h"
#include "platform.h"

void platform_init(void) { }
void platform_log(const char *fmt, ...)
{
  char b[160]; va_list ap; va_start(ap, fmt); vsnprintf(b, sizeof b, fmt, ap); va_end(ap);
  log_printf("%s\n", b);
}
void platform_led(int on) { (void)on; }
void platform_finish(int ok) { test_check(ok, "app self-check"); test_done(); }
