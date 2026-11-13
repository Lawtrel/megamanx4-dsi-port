#ifndef X4_SPRITE_H
#define X4_SPRITE_H
#include <stdint.h>
#include <stddef.h>
/* Tile records from the original assembly bank: flags, tile, signed x/y.
 * Handles opaque 4bpp character tiles, palette groups, XY flips and clipping.
 * No gameplay, lighting or PS1 packet processing. Buffers must not overlap. */
int x4_sprite_draw(uint16_t *screen, unsigned width, unsigned height,
                   const uint8_t *decoded, size_t length,
                   const uint8_t *records, unsigned count,
                   const uint16_t palette[64], int origin_x, int origin_y,
                   unsigned scale, unsigned mirror);
#endif
