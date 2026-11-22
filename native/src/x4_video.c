#include "x4_video.h"
#include <string.h>
int x4_video_enabled;
#ifdef X4_DEVICE_INDEXED
/* Both consoles use the RGB256 hardware compositor. RGBA is a host diagnostic. */
static const int gpu_indexed=1;
#elif defined(X4_N06_INDEXED)
static int gpu_indexed=1;
#else
static int gpu_indexed;
#endif
int x4_video_is_indexed(void){return gpu_indexed;}
void x4_video_set_indexed(int enabled){
#ifndef X4_DEVICE_INDEXED
 if(gpu_indexed!=!!enabled){gpu_indexed=!!enabled;x4_video_invalidate();}
#else
 (void)enabled;
#endif
}
uint32_t x4_video_generation=1;
typedef struct {
 const uint8_t* source;uint32_t tag,frame,generation,palette_version;unsigned stride;uint16_t palette;
 uint8_t bits,w,h,dirty;
} Tile;
_Static_assert(sizeof(void*)!=4||sizeof(Tile)==32,"ARM tile metadata must occupy one cache line");
typedef struct {const uint16_t* source;uint32_t frame,version,index_version;unsigned colors,gpu_slot,lookup_format;uint8_t swap,dirty,transparent;uint16_t data[256],original[256];uint32_t zero[8];} Palette;
/* ARM32 Tile is one 32-byte cache line; don't split keys across lines. */
static Tile tiles[X4_VIDEO_TILES] __attribute__((aligned(32)));
static Palette palettes[X4_VIDEO_PALETTES];
typedef struct {uint32_t frame,version;unsigned owner,valid;uint8_t dirty;
 uint16_t data[256] __attribute__((aligned(32)));} GpuPalette;
_Static_assert(sizeof(GpuPalette)==544,"GPU palette data stays aligned for DMA");
static GpuPalette gpu_palettes[X4_VIDEO_GPU_PALETTES];
static uint32_t palette_dirty[(X4_VIDEO_GPU_PALETTES+31)/32];
/* Keep the frequently queried keys separate from decoded pixel storage. */
static uint8_t indexed[X4_VIDEO_TILES][256] __attribute__((aligned(32)));
typedef union {uint32_t pairs[256];uint16_t single[256];} RgbaTable;
typedef union {uint16_t pairs[256];uint8_t single[256];} IndexTable;
static RgbaTable rgba_tables[X4_VIDEO_PALETTES];
static IndexTable index_tables[X4_VIDEO_PALETTES];
static uint32_t dirty_words[X4_VIDEO_TILES/32];
static uint16_t rgba[X4_VIDEO_TILES][256] __attribute__((aligned(32)));
static X4VideoCommand commands[X4_VIDEO_COMMANDS];
/* Fixed hash buckets avoid scanning the full cache for every visible tile. */
static int16_t buckets[4096],next[X4_VIDEO_TILES];
static int16_t palette_buckets[256],palette_next[X4_VIDEO_PALETTES];
static unsigned count,frame=1,alpha;static int draw_order;static uint16_t clear_color;static X4VideoStats stats;
static int initialized;
static unsigned eviction_cursor;
/* Content sharing preserves both public pixel representations. Aliases keep
 * repeated source keys fast; a serial rejects aliases after slot eviction. */
static int16_t content_buckets[4096],content_next[X4_VIDEO_TILES];
static uint32_t content_keys[X4_VIDEO_TILES],slot_serial[X4_VIDEO_TILES];
static uint16_t decoded_rgba[256] __attribute__((aligned(32)));
static uint8_t decoded_indexed[256] __attribute__((aligned(4)));
#define X4_VIDEO_ALIASES 4096
static struct {Tile key;uint32_t serial;uint16_t slot;} aliases[X4_VIDEO_ALIASES];
static int16_t alias_buckets[4096],alias_next[X4_VIDEO_ALIASES];
static unsigned alias_cursor;
static X4VideoFault video_fault;
static int fail(unsigned reason){
 ++video_fault.count;video_fault.frame=frame;video_fault.reason=reason;
 video_fault.commands=count;video_fault.current=video_fault.previous=0;
 if(reason==X4_VIDEO_FAULT_TILES)for(unsigned i=0;i<X4_VIDEO_TILES;++i){
  video_fault.current+=tiles[i].source&&tiles[i].frame==frame;
  video_fault.previous+=tiles[i].source&&tiles[i].frame+1==frame;
 }
 if(reason==X4_VIDEO_FAULT_COMMANDS||reason==X4_VIDEO_FAULT_PALETTES||reason==X4_VIDEO_FAULT_TILES||reason==X4_VIDEO_FAULT_GPU_PALETTES)++stats.overflow;
 return 1;
}
X4VideoFault x4_video_last_fault(void){return video_fault;}
static inline __attribute__((always_inline)) int same_key(const Tile* t,const uint8_t* raw,unsigned stride,unsigned bits,unsigned pi,
 unsigned w,unsigned h,uint32_t tag,uint32_t version){
 return t->source==raw&&t->stride==stride&&t->bits==bits&&t->palette==pi&&t->tag==tag&&t->w==w&&t->h==h&&t->generation==x4_video_generation&&t->palette_version==version;
}
/* Keep all source aliases in their hash chains. The old single entry per
 * 512 buckets continually evicted live aliases on aligned atlas rows. */
static int alias_find(unsigned hash,const uint8_t* raw,unsigned stride,unsigned bits,unsigned pi,
 unsigned w,unsigned h,uint32_t tag,uint32_t version){
 int16_t* link=&alias_buckets[hash];
 while(*link>=0){unsigned i=*link;
  if(aliases[i].serial!=slot_serial[aliases[i].slot]){
   *link=alias_next[i];aliases[i].key.source=NULL;continue;
  }
  if(same_key(&aliases[i].key,raw,stride,bits,pi,w,h,tag,version)){
   aliases[i].key.frame=frame;return i;
  }
  link=&alias_next[i];
 }
 return -1;
}

/* Hash every output word so common empty borders do not put hundreds of
 * different atlas tiles in one chain. Full RGBA AND indexed comparisons
 * remain the final authority; a hash collision never permits sharing. */
static uint32_t content_key(void){
 uint32_t hash=2166136261u;
 for(unsigned i=0;i<256;i+=8){uint32_t pairs[4];memcpy(pairs,decoded_rgba+i,sizeof pairs);
  hash=(hash^pairs[0])*16777619u;hash=(hash^pairs[1])*16777619u;
  hash=(hash^pairs[2])*16777619u;hash=(hash^pairs[3])*16777619u;
 }
 return hash;
}
static int last_palette=-1;
static int screen_space,view_x,view_y,view_w=320,view_h=240;
void x4_video_invalidate(void){
 ++x4_video_generation;
 /* Keep textures referenced by the preceding GPU frame alive. A generation
  * key rejects old resources without reusing their VRAM slots immediately. */
 if(!initialized){
#ifndef X4_DEVICE_INDEXED
 memset(content_buckets,0xff,sizeof content_buckets);memset(content_next,0xff,sizeof content_next);memset(aliases,0,sizeof aliases);memset(alias_buckets,0xff,sizeof alias_buckets);memset(alias_next,0xff,sizeof alias_next);
#endif
 memset(tiles,0,sizeof tiles);memset(palettes,0,sizeof palettes);memset(buckets,0xff,sizeof buckets);memset(next,0xff,sizeof next);memset(palette_buckets,0xff,sizeof palette_buckets);memset(palette_next,0xff,sizeof palette_next);}
 for(unsigned i=0;i<X4_VIDEO_PALETTES;++i)palettes[i].frame=0;
 initialized=1;
}
void x4_video_begin(uint16_t clear){
 if(!initialized||!frame)x4_video_invalidate();
 ++frame;count=alpha=0;draw_order=screen_space=0;clear_color=clear;
 last_palette=-1;
 memset(&stats,0,sizeof stats);
}
void x4_video_order(int order){draw_order=order;}
void x4_video_screen_space(int enabled){screen_space=!!enabled;}
void x4_video_viewport(int x,int y,int w,int h){view_x=x;view_y=y;view_w=w;view_h=h;}
void x4_video_get_viewport(int* x,int* y,int* w,int* h){*x=view_x;*y=view_y;*w=view_w;*h=view_h;}
static unsigned palette_hash(const uint16_t* p,unsigned colors){uintptr_t v=(uintptr_t)p;return ((v>>5)^(v>>13)^colors*17u)&255;}
static int palette_slot(const uint16_t* p,unsigned colors){
 if(last_palette>=0&&palettes[last_palette].source==p&&palettes[last_palette].colors==colors&&palettes[last_palette].frame==frame)return last_palette;
 unsigned slot=0,oldest=~0u;
 unsigned hash=palette_hash(p,colors);
 for(int i=palette_buckets[hash];i>=0;i=palette_next[i])if(palettes[i].source==p&&palettes[i].colors==colors){slot=i;goto found;}
 for(unsigned i=0;i<X4_VIDEO_PALETTES;++i){
  if(palettes[i].frame<oldest){oldest=palettes[i].frame;slot=i;}
 }
 if(oldest==frame){fail(X4_VIDEO_FAULT_PALETTES);return -1;}
 if(palettes[slot].source){unsigned oldhash=palette_hash(palettes[slot].source,palettes[slot].colors);
  int16_t* link=&palette_buckets[oldhash];while(*link>=0&&*link!=(int)slot)link=&palette_next[*link];if(*link>=0)*link=palette_next[slot];}
 palette_next[slot]=palette_buckets[hash];palette_buckets[hash]=slot;
found:;
 last_palette=slot;Palette* q=&palettes[slot];if(q->frame==frame)return (int)slot;
 /* A palette is checked again on its first use in each new frame. Most
  * sources are unchanged: compare only their original colors, retaining
  * remapped data, lookup tables and version without rebuilding them. */
 if(q->source==p&&q->colors==colors&&q->lookup_format==(unsigned)gpu_indexed&&!memcmp(q->original,p,colors*sizeof *p)){q->frame=frame;return (int)slot;}
 uint16_t data[256]={0};uint32_t zero[8]={0};unsigned swap=0,transparent=0;
 for(unsigned i=0;i<colors;++i){data[i]=p[i]&32767;if(!p[i]){transparent=1;zero[i>>5]|=1u<<(i&31);if(!swap&&i)swap=i;}}
 /* PS1 assets use zero COLOR, rather than a mandatory transparent index.
  * Swap an opaque index zero to the first transparent index when needed. */
 if(!p[0]||!swap)swap=0;
 if(swap){data[swap]=data[0];data[0]=0;}
 int changed=q->source!=p||q->colors!=colors||q->swap!=swap||memcmp(q->zero,zero,sizeof zero);
 if(changed)++q->index_version;
 if(changed||q->lookup_format!=(unsigned)gpu_indexed||memcmp(q->data,data,sizeof data)){
  ++q->version;memcpy(q->data,data,sizeof data);q->dirty=1;++stats.palette_uploads;
  if(!gpu_indexed||changed||q->lookup_format!=(unsigned)gpu_indexed){
  if(colors==16){
   uint16_t rgb[16];uint8_t indices[16];
   for(unsigned i=0;i<16;++i){rgb[i]=p[i]?(p[i]|0x8000):0;indices[i]=!p[i]?0:!i&&swap?swap:i;}
   for(unsigned i=0;i<256;++i){if(!gpu_indexed)rgba_tables[slot].pairs[i]=rgb[i&15]|((uint32_t)rgb[i>>4]<<16);index_tables[slot].pairs[i]=indices[i&15]|((uint16_t)indices[i>>4]<<8);}
  }else if(colors==256){
   for(unsigned i=0;i<256;++i){if(!gpu_indexed)rgba_tables[slot].single[i]=p[i]?(p[i]|0x8000):0;index_tables[slot].single[i]=!p[i]?0:!i&&swap?swap:i;}
  }
 }
 }
 memcpy(q->original,p,colors*sizeof *p);memcpy(q->zero,zero,sizeof zero);
 q->lookup_format=gpu_indexed;q->transparent=transparent;q->source=p;q->colors=colors;q->swap=swap;q->frame=frame;
 return (int)slot;
}
static unsigned hash_key(const uint8_t* p,unsigned stride,unsigned bits,unsigned palette,unsigned w,unsigned h,uint32_t tag,uint32_t generation,uint32_t version){
 uintptr_t v=(uintptr_t)p;return (unsigned)((v>>3)^(v>>15)^(stride*31u)^(palette*257u)^(bits*7u)^(w*17u)^(h*23u)^tag^(generation*13u)^(version*19u))&4095;
}
/* Color changes allocate a retained hardware CLUT, not new pixel data.
 * Metadata may be reused next frame; physical palettes protect both images. */
static int gpu_palette_slot(unsigned pi){
 Palette* q=&palettes[pi];unsigned slot=q->gpu_slot;
 if(slot<X4_VIDEO_GPU_PALETTES&&gpu_palettes[slot].valid&&gpu_palettes[slot].owner==pi&&gpu_palettes[slot].version==q->version){gpu_palettes[slot].frame=frame;return slot;}
 unsigned oldest=~0u;slot=X4_VIDEO_GPU_PALETTES;
 for(unsigned i=0;i<X4_VIDEO_GPU_PALETTES;++i)if(!gpu_palettes[i].valid){slot=i;break;}
 else if(gpu_palettes[i].frame+1<frame&&gpu_palettes[i].frame<oldest){oldest=gpu_palettes[i].frame;slot=i;}
 if(slot==X4_VIDEO_GPU_PALETTES){fail(X4_VIDEO_FAULT_GPU_PALETTES);return -1;}
 GpuPalette* g=&gpu_palettes[slot];g->valid=1;g->owner=pi;g->version=q->version;g->frame=frame;g->dirty=1;
 memcpy(g->data,q->data,sizeof g->data);q->gpu_slot=slot;palette_dirty[slot>>5]|=1u<<(slot&31);return slot;
}
static void unlink_tile(unsigned slot){
 Tile* t=&tiles[slot];if(!t->source)return;
#ifndef X4_DEVICE_INDEXED
 if(!(t->bits&16)){
  int16_t* link=&content_buckets[content_keys[slot]&4095];
  while(*link>=0&&*link!=(int)slot)link=&content_next[*link];if(*link>=0)*link=content_next[slot];
 }
#endif
 unsigned oldhash=hash_key(t->source,t->stride,t->bits,t->palette,t->w,t->h,t->tag,t->generation,t->palette_version);
 int16_t* link=&buckets[oldhash];while(*link>=0&&*link!=(int)slot)link=&next[*link];if(*link>=0)*link=next[slot];
}
static int indexed_tile(const uint8_t* raw,unsigned stride,unsigned bits,const uint16_t* p,unsigned colors,
 int x,int y,unsigned w,unsigned h,unsigned flip,int semi,uint32_t tag){
 int pi=palette_slot(p,colors);if(pi<0)return 1;
 int physical=gpu_palette_slot(pi);if(physical<0)return 1;
 Palette* q=&palettes[pi];uint32_t version=q->index_version;unsigned key_bits=bits|16;
 unsigned hash=hash_key(raw,stride,key_bits,pi,w,h,tag,x4_video_generation,version),slot=X4_VIDEO_TILES;
 for(int i=buckets[hash];i>=0;i=next[i])if(same_key(&tiles[i],raw,stride,key_bits,pi,w,h,tag,version)){slot=i;break;}
 if(slot==X4_VIDEO_TILES){
  for(unsigned n=0;n<X4_VIDEO_TILES;++n){unsigned i=eviction_cursor++%X4_VIDEO_TILES;if(!tiles[i].source||tiles[i].frame+1<frame){slot=i;break;}}
  if(slot==X4_VIDEO_TILES)return fail(X4_VIDEO_FAULT_TILES);
  unlink_tile(slot);Tile* t=&tiles[slot];t->source=NULL;t->frame=0;
#ifndef X4_DEVICE_INDEXED
  ++slot_serial[slot];
#endif
  if(w!=16||h!=16)memset(indexed[slot],0,sizeof indexed[slot]);
  if(bits==4&&colors==16&&w==16){
   const uint16_t* pairs=index_tables[pi].pairs;
   for(unsigned sy=0;sy<h;++sy)for(unsigned sx=0;sx<8;++sx){unsigned byte=raw[sy*stride+sx];memcpy(&indexed[slot][sy*16+sx*2],&pairs[byte],2);}
  }else if(bits==8&&colors==256){
   const uint8_t* indices=index_tables[pi].single;
   for(unsigned sy=0;sy<h;++sy)for(unsigned sx=0;sx<w;++sx)indexed[slot][sy*16+sx]=indices[raw[sy*stride+sx]];
  }else for(unsigned sy=0;sy<h;++sy)for(unsigned sx=0;sx<w;++sx){
   unsigned index=bits==4?(raw[sy*stride+sx/2]>>((sx&1)*4))&15:raw[sy*stride+sx];
   if(index>=colors)return fail(X4_VIDEO_FAULT_INDEX);
   indexed[slot][sy*16+sx]=!p[index]?0:!index&&q->swap?q->swap:index;
  }
  *t=(Tile){.source=raw,.tag=tag,.generation=x4_video_generation,.palette_version=version,.stride=stride,.palette=pi,.bits=key_bits,.w=w,.h=h,.dirty=1};
  next[slot]=buckets[hash];buckets[hash]=slot;dirty_words[slot>>5]|=1u<<(slot&31);++stats.misses;
 }else ++stats.hits;
 tiles[slot].frame=frame;X4VideoCommand* c=&commands[count++];
 c->order=draw_order;c->screen_space=screen_space;c->x[0]=c->x[3]=x;c->x[1]=c->x[2]=x+w;c->y[0]=c->y[1]=y;c->y[2]=c->y[3]=y+h;
 c->tile=slot;c->palette=physical;c->w=w;c->h=h;c->flip=flip;c->semi=!!semi;c->textured=q->transparent?2:1;
 for(unsigned i=0;i<4;++i)c->color[i]=32767;stats.commands=count;return 0;
}
int x4_video_tile(const uint8_t* raw,unsigned stride,unsigned bits,const uint16_t* p,
 unsigned colors,int x,int y,unsigned w,unsigned h,unsigned flip,int semi,uint32_t tag){
 int vx=screen_space?0:view_x,vy=screen_space?0:view_y,vw=screen_space?320:view_w,vh=screen_space?240:view_h;
 if(x>=vx+vw||y>=vy+vh||x+(int)w<=vx||y+(int)h<=vy)return 0;
 if(!raw||!p||!w||w>16||!h||h>16||(bits!=4&&bits!=8)||!colors||colors>256)return fail(X4_VIDEO_FAULT_ARGUMENTS);
 if(count>=X4_VIDEO_COMMANDS)return fail(X4_VIDEO_FAULT_COMMANDS);
 if(gpu_indexed)return indexed_tile(raw,stride,bits,p,colors,x,y,w,h,flip,semi,tag);
 int pi=palette_slot(p,colors);if(pi<0)return 1;
 uint32_t version=palettes[pi].version;
 unsigned hash=hash_key(raw,stride,bits,pi,w,h,tag,x4_video_generation,version),slot=X4_VIDEO_RGBA_TILES;
 for(int i=buckets[hash];i>=0;i=next[i]){
  Tile* t=&tiles[i];if(same_key(t,raw,stride,bits,pi,w,h,tag,version)){slot=i;break;}
 }
 if(slot==X4_VIDEO_RGBA_TILES){
  int alias=alias_find(hash,raw,stride,bits,pi,w,h,tag,version);
  if(alias>=0){slot=aliases[alias].slot;++stats.hits;++stats.alias_hits;goto ready;}
  if(w!=16||h!=16){memset(decoded_rgba,0,sizeof decoded_rgba);memset(decoded_indexed,0,sizeof decoded_indexed);}
  Palette* q=&palettes[pi];
  if(bits==4&&colors==16&&w==16){
   /* Two original 4bpp pixels per lookup; full tiles need no clearing.
    * memcpy has fixed sizes and becomes aligned stores on ARM9. */
   const uint32_t* pairs=rgba_tables[pi].pairs;const uint16_t* indices=index_tables[pi].pairs;
   for(unsigned sy=0;sy<h;++sy)for(unsigned sx=0;sx<8;++sx){
    unsigned byte=raw[sy*stride+sx];
    memcpy(&decoded_rgba[sy*16+sx*2],&pairs[byte],4);
    memcpy(&decoded_indexed[sy*16+sx*2],&indices[byte],2);
   }
  }else if(bits==8&&colors==256){
   const uint16_t* rgb=rgba_tables[pi].single;const uint8_t* indices=index_tables[pi].single;
   for(unsigned sy=0;sy<h;++sy)for(unsigned sx=0;sx<w;++sx){unsigned byte=raw[sy*stride+sx];decoded_rgba[sy*16+sx]=rgb[byte];decoded_indexed[sy*16+sx]=indices[byte];}
  }else for(unsigned sy=0;sy<h;++sy)for(unsigned sx=0;sx<w;++sx){
   unsigned index=bits==4?(raw[sy*stride+sx/2]>>((sx&1)*4))&15:raw[sy*stride+sx];
   if(index>=colors)return fail(X4_VIDEO_FAULT_INDEX);
   decoded_rgba[sy*16+sx]=p[index]?(p[index]|0x8000):0;
   if(q->zero[index>>5]&(1u<<(index&31)))index=0;else if(!index&&q->swap)index=q->swap;
   decoded_indexed[sy*16+sx]=index;
  }

  uint32_t key=content_key();
  for(int i=content_buckets[key&4095];i>=0;i=content_next[i])if(content_keys[i]==key&&
     !memcmp(decoded_rgba,rgba[i],sizeof decoded_rgba)&&!memcmp(decoded_indexed,indexed[i],sizeof decoded_indexed)){
   slot=i;
   /* At most 1500 commands per draw: 4096 aliases fit both protected draws.
    * Prefer an unused/stale/older alias; never scan this pool on a hit. */
   for(unsigned n=0;n<X4_VIDEO_ALIASES;++n){unsigned a=alias_cursor++%X4_VIDEO_ALIASES;
    if(!aliases[a].key.source||aliases[a].serial!=slot_serial[aliases[a].slot]||aliases[a].key.frame+1<frame){alias=a;break;}}
   if(alias>=0){
    if(aliases[alias].key.source){Tile* k=&aliases[alias].key;
     unsigned oldhash=hash_key(k->source,k->stride,k->bits,k->palette,k->w,k->h,k->tag,k->generation,k->palette_version);
     int16_t* link=&alias_buckets[oldhash];while(*link>=0&&*link!=alias)link=&alias_next[*link];if(*link>=0)*link=alias_next[alias];
    }
    aliases[alias].key=(Tile){.source=raw,.tag=tag,.frame=frame,.generation=x4_video_generation,.palette_version=version,.stride=stride,.palette=pi,.bits=bits,.w=w,.h=h};
    aliases[alias].slot=slot;aliases[alias].serial=slot_serial[slot];
    alias_next[alias]=alias_buckets[hash];alias_buckets[hash]=alias;
   }
   ++stats.hits;++stats.shared;goto ready;
  }
  /* A clock replacement avoids scanning 1024 entries on every miss. Both
   * the current and preceding GPU frame remain protected from eviction. */
  for(unsigned n=0;n<X4_VIDEO_RGBA_TILES;++n){unsigned i=eviction_cursor++%X4_VIDEO_RGBA_TILES;
   if(!tiles[i].source||tiles[i].frame+1<frame){slot=i;break;}}
  if(slot==X4_VIDEO_RGBA_TILES)return fail(X4_VIDEO_FAULT_TILES);
  unlink_tile(slot);Tile* t=&tiles[slot];

  *t=(Tile){.source=raw,.tag=tag,.generation=x4_video_generation,.palette_version=version,.stride=stride,.palette=pi,.bits=bits,.w=w,.h=h,.dirty=1};
  ++slot_serial[slot];
  memcpy(rgba[slot],decoded_rgba,sizeof decoded_rgba);memcpy(indexed[slot],decoded_indexed,sizeof decoded_indexed);
  content_keys[slot]=key;content_next[slot]=content_buckets[key&4095];content_buckets[key&4095]=slot;
  dirty_words[slot>>5]|=1u<<(slot&31);
  next[slot]=buckets[hash];buckets[hash]=slot;++stats.misses;
 }else ++stats.hits;
ready:
 tiles[slot].frame=frame;
 X4VideoCommand* c=&commands[count++];
 c->order=draw_order;
 c->screen_space=screen_space;
 c->x[0]=c->x[3]=x;c->x[1]=c->x[2]=x+w;c->y[0]=c->y[1]=y;c->y[2]=c->y[3]=y+h;
 c->tile=slot;c->palette=pi;c->w=w;c->h=h;c->flip=flip;c->semi=!!semi;
 /* No transparent colors means index zero must be drawn normally. */
 c->textured=palettes[pi].transparent?2:1;
 for(unsigned i=0;i<4;++i)c->color[i]=32767;
 stats.commands=count;return 0;
}
int x4_video_quad(const int* x,const int* y,const uint16_t* colors,int semi){
 if(count>=X4_VIDEO_COMMANDS)return fail(X4_VIDEO_FAULT_COMMANDS);
 X4VideoCommand* c=&commands[count++];memset(c,0,sizeof *c);c->semi=!!semi;
 c->order=draw_order;
 c->screen_space=screen_space;
 for(unsigned i=0;i<4;++i){c->x[i]=x[i];c->y[i]=y[i];c->color[i]=colors[i]&32767;}
 stats.commands=count;return 0;
}
void x4_video_fade(unsigned a){alpha=a>255?255:a;}
const X4VideoCommand* x4_video_commands(unsigned* n){*n=count;return commands;}
unsigned x4_video_dirty_tiles(uint16_t* slots){
 unsigned count=0;
 for(unsigned i=0;i<X4_VIDEO_TILES/32;++i){uint32_t word=dirty_words[i];while(word){unsigned bit=__builtin_ctz(word);slots[count++]=i*32+bit;word&=word-1;}}
 return count;
}
const uint8_t* x4_video_texture(unsigned slot,int* dirty){Tile* t=&tiles[slot];*dirty=t->dirty;t->dirty=0;dirty_words[slot>>5]&=~(1u<<(slot&31));if(*dirty)stats.uploaded_bytes+=256;return indexed[slot];}
const uint16_t* x4_video_peek_rgba(unsigned slot){return rgba[slot];}
const uint8_t* x4_video_peek_indexed(unsigned slot){return indexed[slot];}
unsigned x4_video_dirty_palettes(uint16_t* slots){unsigned n=0;for(unsigned i=0;i<(X4_VIDEO_GPU_PALETTES+31)/32;++i){uint32_t word=palette_dirty[i];while(word){unsigned bit=__builtin_ctz(word);slots[n++]=i*32+bit;word&=word-1;}}return n;}
const uint16_t* x4_video_peek_palette(unsigned slot){return gpu_palettes[slot].data;}
void x4_video_command_rgba(const X4VideoCommand* c,uint16_t pixels[256]){
 if(!gpu_indexed){memcpy(pixels,rgba[c->tile],512);return;}
 const uint8_t* indices=indexed[c->tile];const uint16_t* colors=gpu_palettes[c->palette].data;
 for(unsigned y=0;y<16;++y)for(unsigned x=0;x<16;++x){unsigned i=y*16+x,index=indices[i];pixels[i]=x>=c->w||y>=c->h||(!index&&c->textured==2)?0:colors[index]|0x8000;}
}
const uint16_t* x4_video_rgba_texture(unsigned slot,int* dirty){Tile* t=&tiles[slot];*dirty=t->dirty;t->dirty=0;dirty_words[slot>>5]&=~(1u<<(slot&31));if(*dirty)stats.uploaded_bytes+=512;return rgba[slot];}
const uint16_t* x4_video_palette(unsigned slot,int* dirty){if(gpu_indexed){GpuPalette* p=&gpu_palettes[slot];*dirty=p->dirty;p->dirty=0;palette_dirty[slot>>5]&=~(1u<<(slot&31));if(*dirty)stats.uploaded_bytes+=512;return p->data;}Palette* p=&palettes[slot];*dirty=p->dirty;p->dirty=0;if(*dirty)stats.uploaded_bytes+=512;return p->data;}
uint16_t x4_video_clear(void){return clear_color;}
unsigned x4_video_alpha(void){return alpha;}
X4VideoStats x4_video_stats(void){return stats;}
