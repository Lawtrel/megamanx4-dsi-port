/* Native DS geometry submission, independent of the original platform APIs. */
#ifndef X4_VIDEO_H
#define X4_VIDEO_H
#include <stdint.h>
#include <stddef.h>
#define X4_VIDEO_TILES 1024
#define X4_VIDEO_PALETTES 128
#define X4_VIDEO_COMMANDS 1500 /* 6000 vertices, below the DS limit of 6144. */
typedef struct {int16_t x[4],y[4],order;uint16_t color[4],tile,palette;uint8_t w,h,flip,semi,textured,screen_space;} X4VideoCommand;
typedef struct {unsigned commands,hits,misses,palette_uploads,overflow,uploaded_bytes;} X4VideoStats;
extern int x4_video_enabled;
extern uint32_t x4_video_generation;
void x4_video_invalidate(void);
void x4_video_begin(uint16_t clear);
void x4_video_order(int order);
void x4_video_screen_space(int screen_space);
void x4_video_viewport(int x,int y,int w,int h);
void x4_video_get_viewport(int* x,int* y,int* w,int* h);
/* raw must cover h rows of stride bytes; 4bpp tiles start at an even nibble. */
int x4_video_tile(const uint8_t* raw,unsigned stride,unsigned bits,const uint16_t* palette,
 unsigned colors,int x,int y,unsigned w,unsigned h,unsigned flip,int semi,uint32_t tag);
int x4_video_quad(const int* x,const int* y,const uint16_t* colors,int semi);
void x4_video_fade(unsigned alpha);
const X4VideoCommand* x4_video_commands(unsigned* count);
const uint8_t* x4_video_texture(unsigned slot,int* dirty);
const uint16_t* x4_video_rgba_texture(unsigned slot,int* dirty);
const uint16_t* x4_video_palette(unsigned slot,int* dirty);
uint16_t x4_video_clear(void);
unsigned x4_video_alpha(void);
X4VideoStats x4_video_stats(void);
#endif
