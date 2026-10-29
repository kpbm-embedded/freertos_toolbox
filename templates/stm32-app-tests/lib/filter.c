#include "filter.h"

void filter_add(filter_t *f, int32_t s)
{
  if (f->count == FILTER_N) f->sum -= f->buf[f->head]; else f->count++;
  f->buf[f->head] = s;
  f->sum += s;
  f->head = (f->head + 1) % FILTER_N;
}

int32_t filter_avg(const filter_t *f) { return f->count ? (int32_t)(f->sum / (int64_t)f->count) : 0; }
