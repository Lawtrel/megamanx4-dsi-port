#include "x4_video.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static uint8_t raw[4096];static uint16_t palette[256];
int main(void){
 x4_video_set_indexed(1);uint32_t random=0x936ab271;unsigned cases=0;uint16_t decoded[256],slots[X4_VIDEO_TILES];
 for(unsigned bits=4;bits<=8;bits+=4)for(unsigned h=1;h<=16;++h)for(unsigned w=1;w<=16;++w){
  unsigned colors=bits==4?16:256;x4_video_begin(0);
  for(unsigned i=0;i<colors;++i){random=random*1664525+1013904223;palette[i]=i%11?random&32767:0;}palette[0]=0x8000;
  for(unsigned i=0;i<sizeof raw;++i){random=random*1664525+1013904223;raw[i]=random>>24;}
  assert(!x4_video_tile(raw,128,bits,palette,colors,0,0,w,h,0,0,++cases));
  unsigned n;const X4VideoCommand* c=x4_video_commands(&n);assert(n==1);x4_video_command_rgba(c,decoded);
  for(unsigned y=0;y<16;++y)for(unsigned x=0;x<16;++x){unsigned expected=0;if(x<w&&y<h){unsigned index=bits==4?(raw[y*128+x/2]>>((x&1)*4))&15:raw[y*128+x];expected=palette[index]?palette[index]|0x8000:0;}assert(decoded[y*16+x]==expected);}
  int dirty;unsigned count=x4_video_dirty_tiles(slots);for(unsigned i=0;i<count;++i)x4_video_texture(slots[i],&dirty);
  uint16_t palettes[X4_VIDEO_GPU_PALETTES];count=x4_video_dirty_palettes(palettes);for(unsigned i=0;i<count;++i)x4_video_palette(palettes[i],&dirty);
 }
 assert(cases==512);
 x4_video_begin(0);x4_video_begin(0);memset(raw,0x11,sizeof raw);memset(palette,0,sizeof palette);palette[1]=31;
 assert(!x4_video_tile(raw,128,4,palette,16,0,0,16,16,0,0,10000));unsigned n;const X4VideoCommand* c=x4_video_commands(&n);X4VideoCommand old=c[0];
 int dirty;x4_video_texture(old.tile,&dirty);assert(dirty);palette[1]=992;x4_video_begin(0);
 assert(!x4_video_tile(raw,128,4,palette,16,0,0,16,16,0,0,10000));assert(c[0].tile==old.tile&&c[0].palette!=old.palette&&!x4_video_stats().misses);
 x4_video_command_rgba(&old,decoded);assert(decoded[0]==0x801f);x4_video_command_rgba(c,decoded);assert(decoded[0]==0x83e0);
 assert(!x4_video_dirty_tiles(slots));
 /* Transparency remapping changes indices and protects the previous data. */
 palette[1]=0;x4_video_begin(0);assert(!x4_video_tile(raw,128,4,palette,16,0,0,16,16,0,0,10000));assert(c[0].tile!=old.tile);
 x4_video_command_rgba(c,decoded);assert(!decoded[0]);
 /* Indexed capacity covers 2048 genuinely distinct source/tag keys. */
 x4_video_begin(0);x4_video_begin(0);memset(raw,0x11,sizeof raw);palette[1]=31;
 for(unsigned i=0;i<1000;++i)assert(!x4_video_tile(raw,128,4,palette,16,0,0,16,16,0,0,20000+i));
 x4_video_begin(0);
 for(unsigned i=0;i<1048;++i)assert(!x4_video_tile(raw,128,4,palette,16,0,0,16,16,0,0,30000+i));
 assert(x4_video_tile(raw,128,4,palette,16,0,0,16,16,0,0,40000));X4VideoFault fault=x4_video_last_fault();assert(fault.reason==X4_VIDEO_FAULT_TILES&&fault.current==1048&&fault.previous==1000);
 x4_video_begin(0);assert(!x4_video_tile(raw,128,4,palette,16,0,0,16,16,0,0,40000));
 puts("PASS 2048 indexed slots, unchanged preceding pixels and genuine capacity limit");
 x4_video_set_indexed(0);x4_video_begin(0);palette[1]=992;
 assert(!x4_video_tile(raw,128,4,palette,16,0,0,16,16,0,0,50000));assert(x4_video_rgba_texture(c[0].tile,&dirty)[0]==0x83e0);
 x4_video_set_indexed(1);x4_video_begin(0);assert(!x4_video_tile(raw,128,4,palette,16,0,0,16,16,0,0,50000));x4_video_command_rgba(c,decoded);assert(decoded[0]==0x83e0);
 puts("PASS switching the diagnostic reference format rebuilds palette lookup tables");
 puts("PASS 512 indexed scalar comparisons, opaque black, retained physical palettes and palette-only animations without pixel uploads");
}
