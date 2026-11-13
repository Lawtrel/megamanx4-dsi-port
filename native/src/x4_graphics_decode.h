#ifndef X4_GRAPHICS_DECODE_H
#define X4_GRAPHICS_DECODE_H
#include <stddef.h>
#include <stdint.h>

typedef enum {
    X4_DECODE_OK = 0,
    X4_DECODE_TRUNCATED,
    X4_DECODE_OUTPUT_FULL,
    X4_DECODE_INVALID_REFERENCE,
    X4_DECODE_INVALID_RUN
} X4DecodeStatus;

/* Reimplementation of SLUS-00561 0x80016FF4..0x800170AF.
 * Input/output are non-overlapping byte buffers; output is little-endian.
 * No CPU register state, PS1 addresses, runtime dispatch or peripheral access.
 * On failure output can contain a partial result; discard it. Both counts
 * are optional and report consumed input and produced output, including errors.
 */
X4DecodeStatus x4_graphics_decode(const uint8_t *src, size_t src_size,
                                uint8_t *dst, size_t dst_capacity,
                                size_t *consumed, size_t *written);
#endif
