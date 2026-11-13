#include "x4_graphics_decode.h"

static uint16_t read_le16(const uint8_t *p) {
    return (uint16_t)((uint16_t)p[0] | ((uint16_t)p[1] << 8));
}

X4DecodeStatus x4_graphics_decode(const uint8_t *src, size_t src_size,
                                uint8_t *dst, size_t dst_capacity,
                                size_t *consumed, size_t *written) {
    size_t input = 0, output = 0;
    X4DecodeStatus result = X4_DECODE_OK;
    for (;;) {
        if (src_size - input < 2) { result = X4_DECODE_TRUNCATED; break; }
        uint16_t control = read_le16(src + input);
        input += 2;
        for (unsigned mask = 0x8000; mask; mask >>= 1) {
            if (src_size - input < 2) { result = X4_DECODE_TRUNCATED; goto done; }
            uint16_t value = read_le16(src + input);
            input += 2;
            if (!(control & mask)) {
                if (dst_capacity - output < 2) { result = X4_DECODE_OUTPUT_FULL; goto done; }
                dst[output++] = (uint8_t)value;
                dst[output++] = (uint8_t)(value >> 8);
                continue;
            }
            size_t count = value >> 11;
            size_t distance = value & 0x7ff;
            if (!count) {
                if (src_size - input < 2) { result = X4_DECODE_TRUNCATED; goto done; }
                count = read_le16(src + input);
                input += 2;
            }
            if (!(count | distance)) goto done;
            /* Original code assumes valid data. Reject its zero-count
             * underflow case and out-of-buffer reads for malformed assets. */
            if (!count) { result = X4_DECODE_INVALID_RUN; goto done; }
            if (distance > output / 2) { result = X4_DECODE_INVALID_REFERENCE; goto done; }
            if (count > (dst_capacity - output) / 2) { result = X4_DECODE_OUTPUT_FULL; goto done; }
            while (count--) {
                uint8_t lo = 0, hi = 0;
                if (distance) {
                    lo = dst[output - distance * 2];
                    hi = dst[output - distance * 2 + 1];
                }
                dst[output++] = lo;
                dst[output++] = hi;
            }
        }
    }
done:
    if (consumed) *consumed = input;
    if (written) *written = output;
    return result;
}
