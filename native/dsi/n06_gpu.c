/* DS 3D core used as a native 2D tile/sprite compositor. No emulated GPU. */
#include <nds.h>
#include <string.h>
#include "x4_video.h"
static uint16_t sorted[X4_VIDEO_COMMANDS],scratch[X4_VIDEO_COMMANDS];
/* Worst case remains bounded by the old 21 words/quad. Pack four opcodes
 * together across vertices and retain unchanged color/texture/polygon state.
 * DMA owns this buffer until glCallList returns; no END (libnds restriction). */
static uint32_t list[1+21*X4_VIDEO_COMMANDS] __attribute__((aligned(32)));
static unsigned used,opcode_word,opcode_lane,last_words;
static uint16_t upload_slots[X4_VIDEO_TILES];
static unsigned prepared,prepared_clear;
static unsigned upload_batches,uploaded_tiles;
unsigned x4_gpu_upload_batches(void){return upload_batches;}
unsigned x4_gpu_upload_tiles(void){return uploaded_tiles;}
static void emit(unsigned opcode,uint32_t a,uint32_t b,unsigned parameters){
 if(opcode_lane==4){opcode_word=used++;list[opcode_word]=0;opcode_lane=0;}
 list[opcode_word]|=opcode<<(opcode_lane++*8);
 if(parameters)list[used++]=a;
 if(parameters==2)list[used++]=b;
}
unsigned x4_gpu_words(void){return last_words;}
/* Stable descending ordering-table order, using a bounded radix sort. */
static void sort_commands(const X4VideoCommand* commands,unsigned count){
 unsigned buckets[256],offsets[256];for(unsigned i=0;i<count;++i)sorted[i]=i;
 for(unsigned pass=0;pass<2;++pass){memset(buckets,0,sizeof buckets);
  for(unsigned i=0;i<count;++i){unsigned key=65535u-(uint16_t)(commands[sorted[i]].order+32768);++buckets[(key>>(pass*8))&255];}
  unsigned n=0;for(unsigned i=0;i<256;++i){offsets[i]=n;n+=buckets[i];}
  for(unsigned i=0;i<count;++i){unsigned key=65535u-(uint16_t)(commands[sorted[i]].order+32768);scratch[offsets[(key>>(pass*8))&255]++]=sorted[i];}
  memcpy(sorted,scratch,count*sizeof *sorted);
 }
}
void x4_gpu_init(void){
 videoSetMode(MODE_0_3D);vramSetBankA(VRAM_A_TEXTURE_SLOT0);vramSetBankB(VRAM_B_TEXTURE_SLOT1);
 vramSetBankC(VRAM_C_TEXTURE_SLOT2);vramSetBankD(VRAM_D_TEXTURE_SLOT3);
 glInit();glEnable(GL_TEXTURE_2D|GL_BLEND);glDisable(GL_ANTIALIAS|GL_OUTLINE);
 glViewport(0,0,255,191);glClearDepth(GL_MAX_DEPTH);glClearPolyID(63);
 glMatrixMode(GL_PROJECTION);glLoadIdentity();glOrthof32(32,288,216,24,-4096,4096);
 glMatrixMode(GL_MODELVIEW);glLoadIdentity();glMatrixMode(GL_TEXTURE);glLoadIdentity();
 x4_video_invalidate();x4_video_viewport(32,24,256,192);x4_video_enabled=1;
}
void x4_gpu_prepare(void){
 /* CPU-only work runs before waiting for VBlank. No hardware or texture
  * ownership changes here; the preceding GPU frame may still be rendering. */
 unsigned gain=255-x4_video_alpha(),clear=x4_video_clear();
 prepared_clear=RGB15((clear&31)*gain/255,((clear>>5)&31)*gain/255,((clear>>10)&31)*gain/255);
 unsigned count;const X4VideoCommand* commands=x4_video_commands(&count);
 sort_commands(commands,count);
 used=1;opcode_lane=4;
 unsigned last_texture=~0u,last_poly=~0u,last_color=~0u;
 for(unsigned i=0;i<count;++i){
  const X4VideoCommand* c=&commands[sorted[i]];unsigned slot=c->tile;
  unsigned texture=c->textured?(slot*64)|(1<<20)|(1<<23)|(GL_RGBA<<26):0;
  unsigned poly=POLY_ALPHA(c->semi?16:31)|POLY_CULL_NONE|POLY_ID(c->semi?(i%62)+1:0);
  if(texture!=last_texture){emit(FIFO_TEX_FORMAT,texture,0,1);last_texture=texture;}
  if(poly!=last_poly){emit(FIFO_POLY_FORMAT,poly,0,1);last_poly=poly;}
  emit(FIFO_BEGIN,GL_QUADS,0,1);
  int u=0,v=0;
  int us[4]={u,u+c->w,u+c->w,u},vs[4]={v,v,v+c->h,v+c->h};
  if(c->flip&1)for(unsigned j=0;j<4;++j)us[j]=2*u+c->w-us[j];
  if(c->flip&2)for(unsigned j=0;j<4;++j)vs[j]=2*v+c->h-vs[j];
  for(unsigned j=0;j<4;++j){unsigned color=c->color[j];
   if(gain!=255)color=RGB15((color&31)*gain/255,((color>>5)&31)*gain/255,((color>>10)&31)*gain/255);
   int x=c->x[j],y=c->y[j];if(c->screen_space){x=32+x*4/5;y=24+y*4/5;}
   if(color!=last_color){emit(FIFO_COLOR,color,0,1);last_color=color;}
   emit(FIFO_TEX_COORD,(uint16_t)(us[j]<<4)|((uint32_t)(uint16_t)(vs[j]<<4)<<16),0,1);
   emit(j?FIFO_VERTEX_XY:FIFO_VERTEX16,(uint16_t)x|((uint32_t)(uint16_t)y<<16),i,j?1:2);
  }
 }
 last_words=used-1;
 prepared=1;
}
void x4_gpu_present(void){
 if(!prepared)x4_gpu_prepare(); /* Small frontend frames retain this shortcut. */
 unsigned uploads=x4_video_dirty_tiles(upload_slots);
 upload_batches=uploaded_tiles=0;
 /* Skip texture-bank remapping entirely on cache-only frames. Otherwise
  * preserve the existing VBlank transfer and two-frame slot protection. */
 if(uploads){
  vramSetBankA(VRAM_A_LCD);vramSetBankB(VRAM_B_LCD);vramSetBankC(VRAM_C_LCD);vramSetBankD(VRAM_D_LCD);
  for(unsigned j=0;j<uploads;){unsigned i=upload_slots[j++];
   int dirty;const uint16_t* data=x4_video_rgba_texture(i,&dirty);if(!dirty)continue;
   unsigned tiles=1;
   /* Slots are reported in ascending order; both RAM pixels and LCD VRAM
    * are contiguous within a bank. Merge only adjacent dirty slots, never
    * cross a bank or include a clean/protected slot. Consume each flag. */
   while(j<uploads&&upload_slots[j]==i+tiles&&(i+tiles)/256==i/256){
    x4_video_rgba_texture(upload_slots[j++],&dirty);++tiles;
   }
   unsigned bank_index=i/256,offset=(i%256)*512;
   uint8_t* bank=bank_index==0?(uint8_t*)VRAM_A:bank_index==1?(uint8_t*)VRAM_B:bank_index==2?(uint8_t*)VRAM_C:(uint8_t*)VRAM_D;
   DC_FlushRange(data,tiles*512);dmaCopyWords(3,data,bank+offset,tiles*512);
   ++upload_batches;uploaded_tiles+=tiles;
  }
  vramSetBankA(VRAM_A_TEXTURE_SLOT0);vramSetBankB(VRAM_B_TEXTURE_SLOT1);vramSetBankC(VRAM_C_TEXTURE_SLOT2);vramSetBankD(VRAM_D_TEXTURE_SLOT3);
 }
 glClearColor(prepared_clear&31,(prepared_clear>>5)&31,(prepared_clear>>10)&31,31);
 glMatrixMode(GL_MODELVIEW);glLoadIdentity();
 if(used>1){list[0]=last_words;glCallList(list);}
 glFlush(GL_TRANS_MANUALSORT);
 prepared=0;
}
