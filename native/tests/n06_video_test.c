#include "x4_video.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static uint8_t raw[4096];static uint16_t palette[256];
static uint8_t unique_tiles[2048][128];
int main(void){
 for(unsigned i=0;i<2048;++i){memset(unique_tiles[i],0x11,128);unique_tiles[i][0]=i;unique_tiles[i][1]=i>>8;}

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
 for(unsigned i=0;i<512;++i)assert(!x4_video_tile(unique_tiles[i],8,4,palette,16,0,0,16,16,0,0,10000+i));
 x4_video_begin(0);
 for(unsigned i=0;i<512;++i)assert(!x4_video_tile(unique_tiles[512+i],8,4,palette,16,0,0,16,16,0,0,20000+i));
 assert(x4_video_tile(unique_tiles[1024],8,4,palette,16,0,0,16,16,0,0,30000));assert(x4_video_stats().overflow==1);
 X4VideoFault fault=x4_video_last_fault();assert(fault.reason==X4_VIDEO_FAULT_TILES&&fault.current==512&&fault.previous==512);
 uint16_t protected_slots[512];for(unsigned i=0;i<512;++i)protected_slots[i]=c[i].tile;
 x4_video_begin(0);
 for(unsigned i=0;i<512;++i)assert(!x4_video_tile(unique_tiles[1024+i],8,4,palette,16,0,0,16,16,0,0,30000+i));
 for(unsigned i=0;i<512;++i){assert(!x4_video_tile(unique_tiles[512+i],8,4,palette,16,0,0,16,16,0,0,20000+i));assert(c[512+i].tile==protected_slots[i]);}
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
 /* Compare fast 4bpp pairs and 8bpp lookup paths against the scalar rule,
  * including every size, row padding, transparent colors and opaque black. */
 uint32_t random=0x9e3779b9;unsigned cases=0;uint16_t slots[X4_VIDEO_TILES];
 for(unsigned bits=4;bits<=8;bits+=4)for(unsigned h=1;h<=16;++h)for(unsigned w=1;w<=16;++w){
  x4_video_begin(0);unsigned colors=bits==4?16:256,swap=0;
  for(unsigned i=0;i<colors;++i){random=random*1664525+1013904223;palette[i]=i%11?random&32767:0;}
  palette[0]=0x8000;
  for(unsigned i=1;i<colors;++i)if(!palette[i]){swap=i;break;}
  for(unsigned i=0;i<sizeof raw;++i){random=random*1664525+1013904223;raw[i]=random>>24;}
  assert(!x4_video_tile(raw,128,bits,palette,colors,0,0,w,h,0,0,40000+cases++));
  unsigned slot=c[0].tile;unsigned pending=x4_video_dirty_tiles(slots);assert(pending&&pending<=X4_VIDEO_TILES);
  const uint16_t* rgba=x4_video_rgba_texture(slot,&dirty);const uint8_t* indices=x4_video_texture(slot,&dirty);
  for(unsigned y=0;y<16;++y)for(unsigned x=0;x<16;++x){
   unsigned value=0,index=0;
   if(x<w&&y<h){index=bits==4?(raw[y*128+x/2]>>((x&1)*4))&15:raw[y*128+x];
    value=palette[index]?palette[index]|0x8000:0;index=!palette[index]?0:!index&&swap?swap:index;}
   assert(rgba[y*16+x]==value&&indices[y*16+x]==index);
  }
  for(unsigned i=0;i<pending;++i)x4_video_rgba_texture(slots[i],&dirty);
  assert(!x4_video_dirty_tiles(slots));
 }
 assert(cases==512);
 /* Raw-palette snapshots must not confuse the high alpha bit, transparent
  * black and opaque black, or create new tiles for unchanged colors. */
 x4_video_begin(0);x4_video_begin(0);memset(raw,0x11,sizeof raw);memset(palette,0,sizeof palette);palette[1]=31;
 assert(!x4_video_tile(raw,128,4,palette,16,0,0,16,16,0,0,60000));
 unsigned saved_slot=c[0].tile;x4_video_rgba_texture(saved_slot,&dirty);
 for(unsigned i=0;i<8;++i){x4_video_begin(0);assert(!x4_video_tile(raw,128,4,palette,16,0,0,16,16,0,0,60000));assert(c[0].tile==saved_slot&&!x4_video_stats().palette_uploads);}
 palette[1]|=0x8000;x4_video_begin(0);assert(!x4_video_tile(raw,128,4,palette,16,0,0,16,16,0,0,60000));
 assert(c[0].tile==saved_slot&&!x4_video_stats().palette_uploads);
 palette[1]=0;x4_video_begin(0);assert(!x4_video_tile(raw,128,4,palette,16,0,0,16,16,0,0,60000));
 unsigned transparent_slot=c[0].tile;assert(x4_video_rgba_texture(transparent_slot,&dirty)[0]==0);
 palette[1]=0x8000;x4_video_begin(0);assert(!x4_video_tile(raw,128,4,palette,16,0,0,16,16,0,0,60000));
 assert(c[0].tile!=transparent_slot&&x4_video_rgba_texture(c[0].tile,&dirty)[0]==0x8000);
 assert(x4_video_rgba_texture(transparent_slot,&dirty)[0]==0);
 /* Exact output sharing across source addresses/tags and unused CLUT changes.
  * Opaque black, partial tiles and remapped indices remain distinguished. */
 x4_video_begin(0);x4_video_begin(0);memset(raw,0x11,sizeof raw);memset(palette,0,sizeof palette);palette[1]=31;
 assert(!x4_video_tile(raw,128,4,palette,16,0,0,16,16,0,0,80000));unsigned shared_slot=c[0].tile;
 uint8_t twin[128];memset(twin,0x11,sizeof twin);
 assert(!x4_video_tile(twin,8,4,palette,16,0,0,16,16,0,0,80001));assert(c[1].tile==shared_slot&&x4_video_stats().shared>=1);
 x4_video_begin(0);assert(!x4_video_tile(twin,8,4,palette,16,0,0,16,16,0,0,80001));
 assert(c[0].tile==shared_slot&&x4_video_stats().alias_hits==1&&!x4_video_stats().misses);
 palette[9]=123;x4_video_begin(0);assert(!x4_video_tile(twin,8,4,palette,16,0,0,16,16,0,0,80002));assert(c[0].tile==shared_slot);
 /* Distinct indices with equal RGB must NOT share the indexed representation. */
 palette[2]=palette[1];memset(twin,0x22,sizeof twin);x4_video_begin(0);
 assert(!x4_video_tile(twin,8,4,palette,16,0,0,16,16,0,0,80003));assert(c[0].tile!=shared_slot);
 assert(x4_video_texture(c[0].tile,&dirty)[0]==2&&x4_video_texture(shared_slot,&dirty)[0]==1);
 /* A subtle one-pixel change must remain different. */
 x4_video_begin(0);x4_video_begin(0);memset(raw,0x11,sizeof raw);memset(twin,0x11,sizeof twin);twin[0]=0x21;
 assert(!x4_video_tile(raw,128,4,palette,16,0,0,16,16,0,0,81000));unsigned collision_slot=c[0].tile;
 palette[2]=992;x4_video_begin(0);assert(!x4_video_tile(twin,8,4,palette,16,0,0,16,16,0,0,81001));
 assert(c[0].tile!=collision_slot&&x4_video_rgba_texture(c[0].tile,&dirty)[1]==(0x8000|992));
 /* An old source-key alias must not survive reuse of its physical slot. */
 static uint16_t alias_palette[16];for(unsigned i=0;i<16;++i)alias_palette[i]=0x200+i;
 x4_video_begin(0);x4_video_begin(0);memset(raw,0x33,sizeof raw);memset(twin,0x33,sizeof twin);
 assert(!x4_video_tile(raw,128,4,alias_palette,16,0,0,16,16,0,0,82000));
 assert(!x4_video_tile(twin,8,4,alias_palette,16,0,0,16,16,0,0,82001));
 x4_video_begin(0);x4_video_begin(0);for(unsigned i=0;i<16;++i)palette[i]=0x100+i;
 for(unsigned i=0;i<X4_VIDEO_RGBA_TILES;++i)assert(!x4_video_tile(unique_tiles[i],8,4,palette,16,0,0,16,16,0,0,83000+i));
 x4_video_begin(0);x4_video_begin(0);
 assert(!x4_video_tile(twin,8,4,alias_palette,16,0,0,16,16,0,0,82001));
 assert(!x4_video_stats().alias_hits&&x4_video_stats().misses==1);
 assert(x4_video_rgba_texture(c[0].tile,&dirty)[0]==(0x8000|0x203));
 /* Aligned atlas/tag collisions used to displace the sole alias in each
  * 512-entry bucket. All live aliases must survive a new draw together. */
 static uint16_t collision_palette[16];collision_palette[1]=0x345;
 x4_video_begin(0);x4_video_begin(0);memset(twin,0x11,sizeof twin);
 for(unsigned i=0;i<128;++i)assert(!x4_video_tile(twin,8,4,collision_palette,16,0,0,16,16,0,0,90000+i*512));
 x4_video_begin(0);
 for(unsigned i=1;i<128;++i)assert(!x4_video_tile(twin,8,4,collision_palette,16,0,0,16,16,0,0,90000+i*512));
 assert(x4_video_stats().alias_hits==127&&!x4_video_stats().shared&&!x4_video_stats().misses);
 puts("PASS all 127 colliding live aliases reused without pixel conversion");
 puts("PASS one-pixel differences and stale aliases after full slot reuse");
 puts("PASS exact RGBA/indexed sharing, fast aliases and palette-version changes");
 puts("PASS 512 scalar/lookup comparisons and dirty upload snapshots");
 puts("PASS N06 indexed tiles, opaque black, transparency, palettes, pose tags, viewport edges and geometry bounds");
}
