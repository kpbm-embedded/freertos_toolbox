/* heap_4 keeps free blocks in address order and merges neighbours on free. Allocate A, B, C; free B (a hole:
 * two free blocks), free A (A and B merge, still two), free C (everything merges back into one block).
 * Also: minimum-ever free size, allocation/free counters, and a too-large request that fails (hook counted). */
#include "FreeRTOS.h"
#include "task.h"
#include "board.h"

static volatile uint32_t malloc_failures;
void vApplicationMallocFailedHook(void) { malloc_failures++; }

static void show(const char *when, HeapStats_t *s)
{
  vPortGetHeapStats(s);
  log_printf("%-14s free %5u in %u block(s), largest %5u, min-ever %5u, allocs %u frees %u\n", when,
             (unsigned)s->xAvailableHeapSpaceInBytes, (unsigned)s->xNumberOfFreeBlocks,
             (unsigned)s->xSizeOfLargestFreeBlockInBytes, (unsigned)s->xMinimumEverFreeBytesRemaining,
             (unsigned)s->xNumberOfSuccessfulAllocations, (unsigned)s->xNumberOfSuccessfulFrees);
}

static void checker(void *arg)
{
  HeapStats_t s0, s;
  show("start", &s0);
  void *a = pvPortMalloc(1000), *b = pvPortMalloc(2000), *c = pvPortMalloc(1000);
  show("A B C", &s);
  test_check(a && b && c, "three blocks allocated");
  vPortFree(b);
  show("free B", &s);
  test_check(s.xNumberOfFreeBlocks == s0.xNumberOfFreeBlocks + 1, "freeing B leaves a hole (one more free block)");
  test_check(s.xSizeOfLargestFreeBlockInBytes < s.xAvailableHeapSpaceInBytes, "largest block < total free: fragmented");
  vPortFree(a);
  show("free A", &s);
  test_check(s.xNumberOfFreeBlocks == s0.xNumberOfFreeBlocks + 1, "A merged with the hole of B");
  vPortFree(c);
  show("free C", &s);
  test_check(s.xNumberOfFreeBlocks == s0.xNumberOfFreeBlocks, "all merged back");
  test_check(s.xAvailableHeapSpaceInBytes == s0.xAvailableHeapSpaceInBytes, "all memory returned");
  test_check(s.xMinimumEverFreeBytesRemaining <= s0.xAvailableHeapSpaceInBytes - 4000, "minimum-ever free remembers the peak");
  test_check(s.xNumberOfSuccessfulAllocations - s0.xNumberOfSuccessfulAllocations == 3 &&
             s.xNumberOfSuccessfulFrees - s0.xNumberOfSuccessfulFrees == 3, "counters: 3 allocations, 3 frees");
  void *big = pvPortMalloc(configTOTAL_HEAP_SIZE);
  test_check(big == NULL && malloc_failures == 1, "too-large request failed and called the hook once");
  test_done();
}

int main(void)
{
  log_printf("qemu-heap-stats: heap_4 fragmentation and coalescing\n");
  xTaskCreate(checker, "checker", 512, NULL, 1, NULL);
  vTaskStartScheduler();
  return 1;
}
