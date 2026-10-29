/* Small command line on USART1 (PA9 TX / PA10 RX, 115200): interrupt-driven RX -> queue -> CLI task. FreeRTOS+CLI is not bundled with the
 * FreeRTOS-Kernel checkout, so this is a table-driven mini CLI. Commands: help tasks heap uptime led <on|off|toggle> echo <text> reset. */
#include <string.h>
#include "board.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "rtos_glue.h"

static QueueHandle_t rxq;
static uint8_t rxc;

void USART1_IRQHandler(void) { HAL_UART_IRQHandler(&board_uart); }
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *h)
{
  BaseType_t woken = pdFALSE;
  if (h == &board_uart) {
    xQueueSendFromISR(rxq, &rxc, &woken);
    HAL_UART_Receive_IT(h, &rxc, 1);
  }
  portYIELD_FROM_ISR(woken);
}
void HAL_UART_ErrorCallback(UART_HandleTypeDef *h)
{
  __HAL_UART_CLEAR_OREFLAG(h);
  HAL_UART_Receive_IT(h, &rxc, 1);
}

typedef struct { const char *name, *help; void (*fn)(char *arg); } cmd_t;
static void c_help(char *a);
static void c_tasks(char *a) { (void)a; static char buf[320]; vTaskList(buf); rtos_print("Name            State Prio StackLeft Num\r\n%s", buf); }
static void c_heap(char *a) { (void)a; rtos_print("heap free %u B, min ever %u B\r\n", (unsigned)xPortGetFreeHeapSize(), (unsigned)xPortGetMinimumEverFreeHeapSize()); }
static void c_uptime(char *a) { (void)a; rtos_print("%lu ms\r\n", (unsigned long)xTaskGetTickCount()); }
static void c_led(char *a)
{
  if (!strcmp(a, "on")) board_led_set(1); else if (!strcmp(a, "off")) board_led_set(0);
  else if (!strcmp(a, "toggle")) board_led_toggle(); else rtos_print("usage: led on|off|toggle\r\n");
}
static void c_echo(char *a) { rtos_print("%s\r\n", a); }
static void c_reset(char *a) { (void)a; rtos_print("resetting\r\n"); vTaskDelay(pdMS_TO_TICKS(50)); NVIC_SystemReset(); }
static const cmd_t cmds[] = {
  { "help", "list commands", c_help }, { "tasks", "task list with stack high-water marks", c_tasks },
  { "heap", "free heap", c_heap }, { "uptime", "ticks in ms", c_uptime },
  { "led", "led on|off|toggle", c_led }, { "echo", "echo <text>", c_echo }, { "reset", "system reset", c_reset },
};
static void c_help(char *a) { (void)a; for (unsigned i = 0; i < sizeof cmds / sizeof *cmds; i++) rtos_print("  %-8s %s\r\n", cmds[i].name, cmds[i].help); }

static void run(char *line)
{
  char *arg = line;
  while (*arg && *arg != ' ') arg++;
  if (*arg) { *arg++ = 0; while (*arg == ' ') arg++; }
  if (!*line) return;
  for (unsigned i = 0; i < sizeof cmds / sizeof *cmds; i++) if (!strcmp(line, cmds[i].name)) { cmds[i].fn(arg); return; }
  rtos_print("unknown command '%s' (try: help)\r\n", line);
}

static void cli_task(void *a)
{
  (void)a;
  char line[64]; unsigned n = 0; uint8_t c;
  rtos_print("\r\nfreertos uart-cli on %s - type help\r\n> ", BOARD_NAME);
  for (;;) {
    xQueueReceive(rxq, &c, portMAX_DELAY);
    if (c == '\r' || c == '\n') { rtos_print("\r\n"); line[n] = 0; run(line); n = 0; rtos_print("> "); }
    else if ((c == 0x7f || c == 8) && n) { n--; rtos_print("\b \b"); }
    else if (c >= 32 && c < 127 && n < sizeof line - 1) { line[n++] = (char)c; rtos_print("%c", c); }
  }
}
static void led_task(void *a) { (void)a; for (;;) { vTaskDelay(pdMS_TO_TICKS(1000)); board_led_toggle(); } }

int main(void)
{
  board_init();
  rtos_log_init();
  rxq = xQueueCreate(32, 1);
  HAL_NVIC_SetPriority(USART1_IRQn, 6, 0);       /* numerically >= configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY (5) */
  HAL_NVIC_EnableIRQ(USART1_IRQn);
  HAL_UART_Receive_IT(&board_uart, &rxc, 1);
  xTaskCreate(cli_task, "cli", 512, NULL, 2, NULL);
  xTaskCreate(led_task, "led", 128, NULL, 1, NULL);
  vTaskStartScheduler();
  for (;;) {}
}
