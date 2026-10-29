#pragma once
/* HAL <-> FreeRTOS glue and a thread-safe console (USART1 via the board layer). */
void rtos_log_init(void);                          /* call once before the scheduler starts (creates the console mutex) */
void rtos_print(const char *fmt, ...);             /* printf-like, one atomic write, max 160 chars */
void rtos_log(const char *fmt, ...);               /* same with a "[tick ms] " prefix and no CRLF needed */
