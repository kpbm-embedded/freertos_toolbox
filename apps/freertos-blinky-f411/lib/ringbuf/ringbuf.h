/* ringbuf - fixed-size byte FIFO with no allocation and no target dependencies.
 * Example of firmware logic kept in lib/ so it builds for the MCU and for host unit tests
 * (CMake profile "Host-Tests", tests/test_ringbuf.cpp). */
#ifndef RINGBUF_H
#define RINGBUF_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint8_t *buf;
    size_t size;  /* capacity in bytes */
    size_t head;  /* next write index */
    size_t tail;  /* next read index */
    size_t count; /* bytes stored */
} ringbuf_t;

void ringbuf_init(ringbuf_t *rb, uint8_t *storage, size_t size);
bool ringbuf_put(ringbuf_t *rb, uint8_t byte);   /* false when full */
bool ringbuf_get(ringbuf_t *rb, uint8_t *byte);  /* false when empty */
size_t ringbuf_count(const ringbuf_t *rb);
bool ringbuf_full(const ringbuf_t *rb);

#ifdef __cplusplus
}
#endif

#endif /* RINGBUF_H */
