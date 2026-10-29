/* Moving average over the last FILTER_N samples: pure C, host-tested (tests/). */
#pragma once
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
#define FILTER_N 8
typedef struct { int32_t buf[FILTER_N]; int64_t sum; uint32_t count, head; } filter_t;
void filter_add(filter_t *f, int32_t sample);
int32_t filter_avg(const filter_t *f);          /* 0 when empty; average of what is there before FILTER_N samples */
#ifdef __cplusplus
}
#endif
