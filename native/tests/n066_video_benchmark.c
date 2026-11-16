/* Synthetic ARM instruction workloads: cold tiles, warm tiles, unchanged
 * palettes across frames, and animated palettes. Output must match N06.5. */
#include "x4_video.h"
static uint8_t raw[25][2048];static uint8_t aliases_raw[256][2048];static uint16_t palette[256];
__attribute__((noinline,noipa)) void benchmark_start(void){__asm__ volatile("":::"memory");}
__attribute__((noinline,noipa)) void benchmark_end(void){__asm__ volatile("":::"memory");}
uint32_t unit_entry(unsigned bits,unsigned mode){
 uint32_t random=0x936ab271;
 for(unsigned i=0;i<sizeof raw;++i){random=random*1664525+1013904223;((uint8_t*)raw)[i]=random>>24;}
 for(unsigned i=0;i<256;++i)palette[i]=i?0x8000|i:0;
 unsigned colors=bits==4?16:256;x4_video_begin(0);
 if(mode==4){
  for(unsigned i=0;i<sizeof aliases_raw;++i)((uint8_t*)aliases_raw)[i]=0x11;
  for(unsigned i=0;i<256;++i)x4_video_tile(aliases_raw[i],128,bits,palette,colors,0,0,16,16,0,0,0);
  benchmark_start();
  for(unsigned round=0;round<4;++round){x4_video_begin(0);for(unsigned i=0;i<256;++i)x4_video_tile(aliases_raw[i],128,bits,palette,colors,0,0,16,16,0,0,0);}
  benchmark_end();unsigned n;const X4VideoCommand* c=x4_video_commands(&n);uint32_t hash=2166136261u;
  for(unsigned i=0;i<n;++i){uint16_t pixels[256];x4_video_command_rgba(&c[i],pixels);for(unsigned j=0;j<256;++j)hash=(hash^pixels[j])*16777619u;}
  return hash;
 }
 for(unsigned i=0;i<(mode==1?25u:1u);++i)x4_video_tile(raw[i],128,bits,palette,colors,0,0,16,16,0,0,i);
 benchmark_start();
 for(unsigned i=1;i<25;++i){
  if(mode>=2){x4_video_begin(0);if(mode==3)palette[1]^=0x3a5;}
  unsigned tile=mode>=2?0:i;
  x4_video_tile(raw[tile],128,bits,palette,colors,0,0,16,16,0,0,tile);
 }
 benchmark_end();
 unsigned count;const X4VideoCommand* commands=x4_video_commands(&count);uint32_t hash=2166136261u;
 for(unsigned i=count>24?count-24:0;i<count;++i){uint16_t pixels[256];x4_video_command_rgba(&commands[i],pixels);
  for(unsigned j=0;j<256;++j)hash=(hash^pixels[j])*16777619u;
 }
 return hash;
}
