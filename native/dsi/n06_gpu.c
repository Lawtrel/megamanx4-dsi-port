/* DS 3D core used as a native 2D tile/sprite compositor. No emulated GPU. */
#include <nds.h>
#include <string.h>
#include "x4_video.h"
static uint16_t sorted[X4_VIDEO_COMMANDS],scratch[X4_VIDEO_COMMANDS];
/* Per quad: state (4 words), first vertex (5), three XY vertices (4 each). DMA owns
 * this buffer until glCallList returns; no END opcode (libnds restriction). */
static uint32_t list[1+21*X4_VIDEO_COMMANDS] __attribute__((aligned(32)));
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
void x4_gpu_present(void){
 /* Contiguous native RGB555/alpha tiles avoid shared hardware palettes and
  * atlas border sampling. Old frame versions retain their own VRAM slots. */
 vramSetBankA(VRAM_A_LCD);vramSetBankB(VRAM_B_LCD);vramSetBankC(VRAM_C_LCD);vramSetBankD(VRAM_D_LCD);
 for(unsigned i=0;i<X4_VIDEO_TILES;++i){
  int dirty;const uint16_t* data=x4_video_rgba_texture(i,&dirty);if(!dirty)continue;
  unsigned bank_index=i/256,offset=(i%256)*512;
  uint8_t* bank=bank_index==0?(uint8_t*)VRAM_A:bank_index==1?(uint8_t*)VRAM_B:bank_index==2?(uint8_t*)VRAM_C:(uint8_t*)VRAM_D;
  DC_FlushRange(data,512);dmaCopyWords(3,data,bank+offset,512);
 }
 vramSetBankA(VRAM_A_TEXTURE_SLOT0);vramSetBankB(VRAM_B_TEXTURE_SLOT1);vramSetBankC(VRAM_C_TEXTURE_SLOT2);vramSetBankD(VRAM_D_TEXTURE_SLOT3);
 unsigned gain=255-x4_video_alpha(),clear=x4_video_clear();
 glClearColor((clear&31)*gain/255,((clear>>5)&31)*gain/255,((clear>>10)&31)*gain/255,31);
 glMatrixMode(GL_MODELVIEW);glLoadIdentity();unsigned count;const X4VideoCommand* commands=x4_video_commands(&count);
 sort_commands(commands,count);
 unsigned used=1;
 for(unsigned i=0;i<count;++i){
  const X4VideoCommand* c=&commands[sorted[i]];unsigned slot=c->tile;
  unsigned texture=c->textured?(slot*64)|(1<<20)|(1<<23)|(GL_RGBA<<26):0;
  unsigned poly=POLY_ALPHA(c->semi?16:31)|POLY_CULL_NONE|POLY_ID(c->semi?(i%62)+1:0);
  list[used++]=FIFO_COMMAND_PACK(FIFO_TEX_FORMAT,FIFO_POLY_FORMAT,FIFO_BEGIN,0);
  list[used++]=texture;list[used++]=poly;list[used++]=GL_QUADS;
  int u=0,v=0;
  int us[4]={u,u+c->w,u+c->w,u},vs[4]={v,v,v+c->h,v+c->h};
  if(c->flip&1)for(unsigned j=0;j<4;++j)us[j]=2*u+c->w-us[j];
  if(c->flip&2)for(unsigned j=0;j<4;++j)vs[j]=2*v+c->h-vs[j];
  for(unsigned j=0;j<4;++j){unsigned color=c->color[j];
   if(gain!=255)color=RGB15((color&31)*gain/255,((color>>5)&31)*gain/255,((color>>10)&31)*gain/255);
   int x=c->x[j],y=c->y[j];if(c->screen_space){x=32+x*4/5;y=24+y*4/5;}
   list[used++]=FIFO_COMMAND_PACK(FIFO_COLOR,FIFO_TEX_COORD,j?FIFO_VERTEX_XY:FIFO_VERTEX16,0);
   list[used++]=color;
   list[used++]=(uint16_t)(us[j]<<4)|((uint32_t)(uint16_t)(vs[j]<<4)<<16);
   list[used++]=(uint16_t)x|((uint32_t)(uint16_t)y<<16);
   if(!j)list[used++]=i;
  }
 }
 if(used>1){list[0]=used-1;glCallList(list);}
 glFlush(GL_TRANS_MANUALSORT);
}
