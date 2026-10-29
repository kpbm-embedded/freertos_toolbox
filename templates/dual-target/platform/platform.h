/* The three things the app needs from the hardware. qemu.c and stm32.c implement them. */
#pragma once
void platform_init(void);
void platform_log(const char *fmt, ...) __attribute__((format(printf, 1, 2)));
void platform_led(int on);
void platform_finish(int ok);     /* qemu: report and exit; board: keep running */
