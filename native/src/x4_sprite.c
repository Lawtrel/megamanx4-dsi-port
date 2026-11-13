#include "x4_sprite.h"
int x4_sprite_draw(uint16_t *screen, unsigned width, unsigned height,
                   const uint8_t *decoded, size_t length,
                   const uint8_t *records, unsigned count,
                   const uint16_t palette[64], int origin_x, int origin_y,
                   unsigned scale, unsigned mirror) {
    if (!scale || scale > 2 || length % 128) return 0;
    unsigned tiles = length / 128;
    for (unsigned i=0; i<count; ++i) {
        const uint8_t *r = records + i*4;
        if ((r[1] | ((r[0]&3u)<<8)) >= tiles) return 0;
    }
    for (unsigned i=0; i<count; ++i) {
        const uint8_t *r=records+i*4;
        unsigned tile=r[1]|((r[0]&3u)<<8), row=tile/16;
        unsigned strip_tiles=tiles-row*16;
        if(strip_tiles>16)strip_tiles=16;
        int dx=(int8_t)r[2],dy=(int8_t)r[3];
        if(mirror)dx=-dx-16;
        for(unsigned y=0;y<16;++y)for(unsigned x=0;x<16;++x) {
            unsigned sx=((!!(r[0]&64)) ^ !!mirror)?15-x:x;
            unsigned sy=(r[0]&128)?15-y:y;
            unsigned pixel=sy*(strip_tiles*16)+(tile%16)*16+sx;
            unsigned packed=decoded[row*2048+pixel/2];
            unsigned index=(packed>>((pixel&1)*4))&15;
            uint16_t color=palette[((r[0]>>2)&3)*16+index];
            if(!color)continue;
            for(unsigned yy=0;yy<scale;++yy)for(unsigned xx=0;xx<scale;++xx) {
                int px=origin_x+(dx+(int)x)*(int)scale+(int)xx;
                int py=origin_y+(dy+(int)y)*(int)scale+(int)yy;
                if(px>=0&&py>=0&&(unsigned)px<width&&(unsigned)py<height)
                    screen[(unsigned)py*width+(unsigned)px]=color|0x8000;
            }
        }
    }
    return 1;
}
