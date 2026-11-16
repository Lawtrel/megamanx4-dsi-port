/* Native DS geometry submission, independent of the original platform APIs. */
#ifndef X4_VIDEO_H
#define X4_VIDEO_H
#include <stdint.h>
#include <stddef.h>
#define X4_VIDEO_TILES 2048
#define X4_VIDEO_RGBA_TILES 1024
#define X4_VIDEO_GPU_PALETTES 192
#define X4_VIDEO_PALETTES 128
#define X4_VIDEO_COMMANDS 1500 /* 6000 vertices, below the DS limit of 6144. */
typedef struct {int16_t x[4],y[4],order;uint16_t color[4],tile,palette;uint8_t w,h,flip,semi,textured,screen_space;} X4VideoCommand;
typedef struct {unsigned commands,hits,misses,palette_uploads,overflow,uploaded_bytes,shared,alias_hits;} X4VideoStats;
enum {X4_VIDEO_FAULT_ARGUMENTS=1,X4_VIDEO_FAULT_COMMANDS,X4_VIDEO_FAULT_PALETTES,X4_VIDEO_FAULT_TILES,X4_VIDEO_FAULT_INDEX,X4_VIDEO_FAULT_GPU_PALETTES};
typedef struct {unsigned count,frame,reason,commands,current,previous;} X4VideoFault;
X4VideoFault x4_video_last_fault(void);
extern int x4_video_enabled;
extern uint32_t x4_video_generation;
void x4_video_set_indexed(int enabled);
int x4_video_is_indexed(void);
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
/* Read pixels for cache maintenance without consuming an upload. */
const uint16_t* x4_video_peek_rgba(unsigned slot);
const uint8_t* x4_video_peek_indexed(unsigned slot);
unsigned x4_video_dirty_palettes(uint16_t* slots);
const uint16_t* x4_video_peek_palette(unsigned slot);
/* Diagnostic reconstruction from the actual command's retained palette. */
void x4_video_command_rgba(const X4VideoCommand* command,uint16_t pixels[256]);
typedef struct {unsigned waits,start_max,restore_max,late,status,underflows,overflows,total_late,total_waits;} X4GpuTransferStats;
X4GpuTransferStats x4_gpu_transfer_stats(void);
const uint16_t* x4_video_rgba_texture(unsigned slot,int* dirty);
/* Snapshot of unconsumed uploads; callers provide X4_VIDEO_TILES slots. */
unsigned x4_video_dirty_tiles(uint16_t* slots);
const uint16_t* x4_video_palette(unsigned slot,int* dirty);
uint16_t x4_video_clear(void);
unsigned x4_video_alpha(void);
X4VideoStats x4_video_stats(void);
#endif
