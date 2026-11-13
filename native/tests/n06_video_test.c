#include "x4_video.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static uint8_t raw[4096];static uint16_t palette[256];
int main(void){
 x4_video_invalidate();for(unsigned i=0;i<256;++i)palette[i]=0x8000|i;
 palette[5]=0;raw[0]=0x50;raw[128]=0x21;
 x4_video_begin(0x1234);x4_video_order(73);
 assert(!x4_video_tile(raw,128,4,palette,16,304,224,16,16,3,0,0));
 unsigned n;const X4VideoCommand* c=x4_video_commands(&n);assert(n==1&&c[0].x[2]==320&&c[0].y[2]==240&&c[0].order==73&&c[0].flip==3);
 int dirty;const uint8_t* pixels=x4_video_texture(c[0].tile,&dirty);assert(dirty&&pixels[0]==5&&pixels[1]==0&&pixels[16]==1&&pixels[17]==2);
 const uint16_t* colors=x4_video_palette(c[0].palette,&dirty);assert(dirty&&colors[0]==0&&colors[5]==0&&colors[1]==1);
 const uint16_t* rgb=x4_video_rgba_texture(c[0].tile,&dirty);assert(rgb[0]==0x8000&&rgb[1]==0&&rgb[16]==0x8001);
 /* An opaque black at index zero must survive; transparent index 5 is remapped. */
 assert(c[0].textured==2);assert(!x4_video_tile(raw,128,4,palette,16,-16,0,16,16,0,0,0));
 x4_video_commands(&n);assert(n==1);
 x4_video_begin(0);assert(!x4_video_tile(raw,128,4,palette,16,0,0,16,16,0,0,0));
 assert(x4_video_stats().hits==1&&x4_video_stats().misses==0);x4_video_texture(c[0].tile,&dirty);assert(!dirty);
 /* Palette animation gets a new RGB texture; the preceding frame retains its colors. */
 palette[1]=0x8123;x4_video_begin(0);assert(!x4_video_tile(raw,128,4,palette,16,0,0,16,16,0,0,0));
 assert(x4_video_stats().misses==1);colors=x4_video_palette(c[0].palette,&dirty);assert(dirty&&colors[1]==0x123);
 rgb=x4_video_rgba_texture(c[0].tile,&dirty);assert(rgb[16]==0x8123);
 /* A changed transparent index invalidates pixels and unlinks old hash chains. */
 palette[5]=0x8005;palette[7]=0;raw[0]=0x70;x4_video_begin(0);
 assert(!x4_video_tile(raw,128,4,palette,16,0,0,16,16,0,0,0));assert(x4_video_stats().misses==1);
 pixels=x4_video_texture(c[0].tile,&dirty);assert(dirty&&pixels[0]==7&&pixels[1]==0);
 /* Reusing pose memory with a new content tag cannot reuse stale texture data. */
 raw[0]=0x21;x4_video_begin(0);assert(!x4_video_tile(raw,128,4,palette,16,0,0,16,16,0,0,123));
 pixels=x4_video_texture(c[0].tile,&dirty);assert(dirty&&pixels[0]==1&&pixels[1]==2);
 unsigned preceding=c[0].tile;uint16_t preceding_color=x4_video_rgba_texture(preceding,&dirty)[0];
 x4_video_invalidate();x4_video_begin(0);raw[0]=0x43;
 assert(!x4_video_tile(raw,128,4,palette,16,0,0,16,16,0,0,123));
 assert(c[0].tile!=preceding&&x4_video_rgba_texture(preceding,&dirty)[0]==preceding_color);
 x4_video_invalidate();x4_video_begin(0);for(unsigned i=0;i<256;++i)palette[i]=0x8000|i;
 raw[0]=200;assert(!x4_video_tile(raw,128,8,palette,256,0,0,16,16,0,0,0));
 pixels=x4_video_texture(c[0].tile,&dirty);assert(dirty&&pixels[0]==200&&c[0].textured==1);
 assert(x4_video_tile(raw,128,8,palette,16,0,0,16,16,0,0,0));
 x4_video_invalidate();x4_video_begin(0);memset(raw,0,sizeof raw);
 for(unsigned i=0;i<X4_VIDEO_COMMANDS;++i)assert(!x4_video_tile(raw,128,4,palette,16,0,0,16,16,0,0,0));
 assert(x4_video_tile(raw,128,4,palette,16,0,0,16,16,0,0,0));assert(x4_video_stats().overflow==1);
 assert(x4_video_stats().commands*4<6144);x4_video_fade(1000);assert(x4_video_alpha()==255);
 /* Saturate both protected frames. Never overwrite the previous frame;
  * reclaim exactly the older half once a new frame begins. */
 x4_video_begin(0);x4_video_begin(0);
 for(unsigned i=0;i<512;++i)assert(!x4_video_tile(raw,128,4,palette,16,0,0,16,16,0,0,10000+i));
 x4_video_begin(0);
 for(unsigned i=0;i<512;++i)assert(!x4_video_tile(raw,128,4,palette,16,0,0,16,16,0,0,20000+i));
 assert(x4_video_tile(raw,128,4,palette,16,0,0,16,16,0,0,30000));assert(x4_video_stats().overflow==1);
 uint16_t protected_slots[512];for(unsigned i=0;i<512;++i)protected_slots[i]=c[i].tile;
 x4_video_begin(0);
 for(unsigned i=0;i<512;++i)assert(!x4_video_tile(raw,128,4,palette,16,0,0,16,16,0,0,30000+i));
 for(unsigned i=0;i<512;++i){assert(!x4_video_tile(raw,128,4,palette,16,0,0,16,16,0,0,20000+i));assert(c[512+i].tile==protected_slots[i]);}
 assert(x4_video_stats().misses==512&&x4_video_stats().hits==512&&!x4_video_stats().overflow);
 /* Interleaved CLUTs exercise hash collisions, reuse and animated versions. */
 static uint16_t pool[X4_VIDEO_PALETTES][16];
 for(unsigned i=0;i<X4_VIDEO_PALETTES;++i)for(unsigned j=0;j<16;++j)pool[i][j]=1+i+j;
 x4_video_begin(0);x4_video_begin(0);raw[0]=0x11;
 for(unsigned i=0;i<X4_VIDEO_PALETTES;++i)assert(!x4_video_tile(raw,128,4,pool[i],16,0,0,16,16,0,0,0));
 for(unsigned i=0;i<X4_VIDEO_PALETTES;++i){unsigned k=(i*73)%X4_VIDEO_PALETTES;assert(!x4_video_tile(raw,128,4,pool[k],16,0,0,16,16,0,0,0));}
 assert(x4_video_stats().hits==X4_VIDEO_PALETTES&&!x4_video_stats().overflow);
 x4_video_begin(0);pool[13][1]=0x4321;
 assert(!x4_video_tile(raw,128,4,pool[13],16,0,0,16,16,0,0,0));
 assert(x4_video_rgba_texture(c[0].tile,&dirty)[0]==0xc321);
 puts("PASS N06 indexed tiles, opaque black, transparency, palettes, pose tags, viewport edges and geometry bounds");
}
