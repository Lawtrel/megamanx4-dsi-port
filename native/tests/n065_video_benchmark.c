/* Synthetic tiles only. Count ARM instructions in x4_video_tile, excluding
 * initialization, palette-table warmup and output verification. Not FPS. */
#include "x4_video.h"
static uint8_t raw[25][2048];static uint16_t palette[256];
__attribute__((noinline,noipa)) void benchmark_start(void){__asm__ volatile("":::"memory");}
__attribute__((noinline,noipa)) void benchmark_end(void){__asm__ volatile("":::"memory");}
uint32_t unit_entry(unsigned bits,unsigned hits){
 uint32_t random=0x936ab271;
 for(unsigned i=0;i<sizeof raw;++i){random=random*1664525+1013904223;((uint8_t*)raw)[i]=random>>24;}
 for(unsigned i=0;i<256;++i)palette[i]=i?0x8000|i:0;
 unsigned colors=bits==4?16:256;x4_video_begin(0);
 for(unsigned i=0;i<(hits?25u:1u);++i)x4_video_tile(raw[i],128,bits,palette,colors,0,0,16,16,0,0,i);
 benchmark_start();
 for(unsigned i=1;i<25;++i)x4_video_tile(raw[i],128,bits,palette,colors,0,0,16,16,0,0,i);
 benchmark_end();
 unsigned count;const X4VideoCommand* commands=x4_video_commands(&count);uint32_t hash=2166136261u;
 for(unsigned i=count-24;i<count;++i){int dirty;const uint16_t* pixels=x4_video_rgba_texture(commands[i].tile,&dirty);
  for(unsigned j=0;j<256;++j)hash=(hash^pixels[j])*16777619u;
 }
 return hash;
}
