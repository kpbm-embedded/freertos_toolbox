# qemu-cli

FreeRTOS+CLI: registered commands (echo, add, tasks, heap, uptime) run from a self-test script; -DCLI_INTERACTIVE=ON for a UART shell.

Runs on qemu `mps2-an385` (Cortex-M3, FreeRTOS ARM_CM3 port, board layer `toolchain/freertos/qemu/`); no board needed.
Features: freertos, cli, freertos-plus, qemu. The program checks itself and ends with `RESULT: PASS` (qemu exit 0) or `RESULT: FAIL` (exit 1).

## Run

```sh
scripts/app run qemu-cli              # build + run in qemu
scripts/app examples test qemu-cli    # the same, as a test (also runs test.sh if present)
```

Interactive shell: `cmake -S examples/freertos/qemu-cli -B /tmp/cli -G Ninja -DCLI_INTERACTIVE=ON && cmake --build /tmp/cli && scripts/app qemu /tmp/cli/qemu-cli.elf` (inside `. toolchain/freertos/env.sh`), then type `help`.

## Output (qemu, 2026-10-29)

```
qemu-cli: FreeRTOS+CLI self-test
> echo hello
hello
CHECK ok  : 'echo hello' answers with 'hello'
> add 2 40
42
CHECK ok  : 'add 2 40' answers with '42'
> add 0x10 -1
15
CHECK ok  : 'add 0x10 -1' answers with '15'
> tasks
Name          State Prio Stack Num
cli            	X	1	820	1
IDLE           	R	0	246	2
Tmr Svc        	B	6	484	3
CHECK ok  : 'tasks' answers with 'cli'
> heap
free 57672, min-ever 57528
CHECK ok  : 'heap' answers with 'free '
> uptime
3 ms
CHECK ok  : 'uptime' answers with ' ms'
> help

help:
 Lists all the registered commands

echo <word>: print the word
add <a> <b>: print a + b
tasks: list the tasks
heap: free heap now and minimum ever
uptime: milliseconds CHECK ok  : 'help' answers with 'add <a> <b>'
> nosuch
Command not recognised.  Enter 'help' to view a list of available commands.

CHECK ok  : 'nosuch' answers with 'not recognised'
> add 1
Incorrect command parameter(s).  Enter "help" to view a list of available commands.

CHECK ok  : 'add 1' answers with 'Incorrect command parameter'
RESULT: PASS (9 checks)
```

On a board: the code uses only the kernel API plus `board.h` (UART log, a timer interrupt, a software interrupt); port the three board calls to run it on an STM32.
