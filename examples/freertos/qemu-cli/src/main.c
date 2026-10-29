/* FreeRTOS+CLI (FreeRTOS_CLI.c, installed by scripts/setup): commands are registered once and run by
 * FreeRTOS_CLIProcessCommand(), which calls a command repeatedly while it returns pdTRUE (multi-part output).
 * The self-test feeds a script of command lines and checks the answers. Built with -DCLI_INTERACTIVE=ON the
 * same commands then stay available on UART0 (qemu: type into the terminal; a board: any serial terminal). */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "FreeRTOS.h"
#include "task.h"
#include "FreeRTOS_CLI.h"
#include "board.h"

static BaseType_t cmd_echo(char *out, size_t len, const char *cmd)
{
  BaseType_t plen;
  const char *p = FreeRTOS_CLIGetParameter(cmd, 1, &plen);
  snprintf(out, len, "%.*s\r\n", (int)plen, p);
  return pdFALSE;
}

static BaseType_t cmd_add(char *out, size_t len, const char *cmd)
{
  BaseType_t l1, l2;
  long a = strtol(FreeRTOS_CLIGetParameter(cmd, 1, &l1), NULL, 0), b = strtol(FreeRTOS_CLIGetParameter(cmd, 2, &l2), NULL, 0);
  snprintf(out, len, "%ld\r\n", a + b);
  return pdFALSE;
}

static BaseType_t cmd_tasks(char *out, size_t len, const char *cmd)
{
  static int part;                       /* two-part output: header, then the table */
  if (part == 0) { snprintf(out, len, "Name          State Prio Stack Num\r\n"); part = 1; return pdTRUE; }
  vTaskListTasks(out, len);
  part = 0;
  return pdFALSE;
}

static BaseType_t cmd_heap(char *out, size_t len, const char *cmd)
{
  snprintf(out, len, "free %u, min-ever %u\r\n", (unsigned)xPortGetFreeHeapSize(), (unsigned)xPortGetMinimumEverFreeHeapSize());
  return pdFALSE;
}

static BaseType_t cmd_uptime(char *out, size_t len, const char *cmd)
{
  snprintf(out, len, "%lu ms\r\n", (unsigned long)(xTaskGetTickCount() * portTICK_PERIOD_MS));
  return pdFALSE;
}

static const CLI_Command_Definition_t commands[] = {
  { "echo", "echo <word>: print the word\r\n", cmd_echo, 1 },
  { "add", "add <a> <b>: print a + b\r\n", cmd_add, 2 },
  { "tasks", "tasks: list the tasks\r\n", cmd_tasks, 0 },
  { "heap", "heap: free heap now and minimum ever\r\n", cmd_heap, 0 },
  { "uptime", "uptime: milliseconds since start\r\n", cmd_uptime, 0 },
};

static void run_line(const char *line, char *all, size_t all_len)    /* every output part of one command */
{
  char *out = FreeRTOS_CLIGetOutputBuffer();
  BaseType_t more;
  all[0] = 0;
  do {
    more = FreeRTOS_CLIProcessCommand(line, out, configCOMMAND_INT_MAX_OUTPUT_SIZE);
    strncat(all, out, all_len - strlen(all) - 1);
  } while (more == pdTRUE);
}

static void cli_task(void *arg)
{
  static char all[1024];
  struct { const char *line, *expect; } script[] = {
    { "echo hello", "hello" }, { "add 2 40", "42" }, { "add 0x10 -1", "15" }, { "tasks", "cli" },
    { "heap", "free " }, { "uptime", " ms" }, { "help", "add <a> <b>" }, { "nosuch", "not recognised" },
    { "add 1", "Incorrect command parameter" },
  };
  for (size_t i = 0; i < sizeof script / sizeof script[0]; i++) {
    run_line(script[i].line, all, sizeof all);
    log_printf("> %s\n%s", script[i].line, all);
    char what[64];
    snprintf(what, sizeof what, "'%s' answers with '%s'", script[i].line, script[i].expect);
    test_check(strstr(all, script[i].expect) != NULL, what);
  }
#ifdef CLI_INTERACTIVE
  log_printf("self-test done (%d failures); UART shell ready, try 'help'\n> ", test_failures());
  char line[80]; size_t n = 0;
  for (;;) {
    int c = board_getc();
    if (c < 0) { vTaskDelay(pdMS_TO_TICKS(10)); continue; }
    if (c == '\r' || c == '\n') {
      line[n] = 0; board_puts("\r\n");
      if (n) { run_line(line, all, sizeof all); board_puts(all); }
      n = 0; board_puts("> ");
    } else if (n < sizeof line - 1) { line[n++] = (char)c; board_putc((char)c); }
  }
#else
  test_done();
#endif
}

int main(void)
{
  log_printf("qemu-cli: FreeRTOS+CLI self-test\n");
  for (size_t i = 0; i < sizeof commands / sizeof commands[0]; i++) FreeRTOS_CLIRegisterCommand(&commands[i]);
  xTaskCreate(cli_task, "cli", 1024, NULL, 1, NULL);
  vTaskStartScheduler();
  return 1;
}
