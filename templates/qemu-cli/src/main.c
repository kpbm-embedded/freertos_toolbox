/* @NAME@: FreeRTOS+CLI. Add a command: write a handler, add a row to commands[]. */
#include <stdio.h>
#include <string.h>
#include "FreeRTOS.h"
#include "task.h"
#include "FreeRTOS_CLI.h"
#include "board.h"

static BaseType_t cmd_status(char *out, size_t len, const char *cmd)
{
  snprintf(out, len, "up %lu ms, %u tasks\r\n", (unsigned long)xTaskGetTickCount(), (unsigned)uxTaskGetNumberOfTasks());
  return pdFALSE;
}

static const CLI_Command_Definition_t commands[] = {
  { "status", "status: uptime and task count\r\n", cmd_status, 0 },
};

static void run(const char *line, char *all, size_t n)
{
  char *out = FreeRTOS_CLIGetOutputBuffer();
  all[0] = 0;
  while (FreeRTOS_CLIProcessCommand(line, out, configCOMMAND_INT_MAX_OUTPUT_SIZE) == pdTRUE) strncat(all, out, n - strlen(all) - 1);
  strncat(all, out, n - strlen(all) - 1);
}

static void console(void *arg)
{
  static char all[512];
  run("status", all, sizeof all);
  log_printf("> status\n%s", all);
  test_check(strstr(all, "tasks") != NULL, "status command answers");
#ifdef CLI_INTERACTIVE
  char line[64]; size_t n = 0;
  board_puts("> ");
  for (;;) {
    int c = board_getc();
    if (c < 0) { vTaskDelay(pdMS_TO_TICKS(10)); continue; }
    if (c == '\r' || c == '\n') { line[n] = 0; board_puts("\r\n"); if (n) { run(line, all, sizeof all); board_puts(all); } n = 0; board_puts("> "); }
    else if (n < sizeof line - 1) { line[n++] = (char)c; board_putc((char)c); }
  }
#else
  test_done();
#endif
}

int main(void)
{
  for (size_t i = 0; i < sizeof commands / sizeof commands[0]; i++) FreeRTOS_CLIRegisterCommand(&commands[i]);
  xTaskCreate(console, "console", 1024, NULL, 1, NULL);
  vTaskStartScheduler();
  return 1;
}
