#include "ringbuf.h"

void ringbuf_init(ringbuf_t *rb, uint8_t *storage, size_t size)
{
    rb->buf = storage;
    rb->size = size;
    rb->head = rb->tail = rb->count = 0;
}

bool ringbuf_put(ringbuf_t *rb, uint8_t byte)
{
    if (rb->count == rb->size) {
        return false;
    }
    rb->buf[rb->head] = byte;
    rb->head = (rb->head + 1) % rb->size;
    rb->count++;
    return true;
}

bool ringbuf_get(ringbuf_t *rb, uint8_t *byte)
{
    if (rb->count == 0) {
        return false;
    }
    *byte = rb->buf[rb->tail];
    rb->tail = (rb->tail + 1) % rb->size;
    rb->count--;
    return true;
}

size_t ringbuf_count(const ringbuf_t *rb) { return rb->count; }

bool ringbuf_full(const ringbuf_t *rb) { return rb->count == rb->size; }
