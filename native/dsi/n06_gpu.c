/* DS 3D core used as a native 2D tile/sprite compositor. No emulated GPU. */
#include <nds.h>
#include <string.h>
#include "x4_video.h"
static uint16_t sorted[X4_VIDEO_COMMANDS];
static uint32_t scratch[X4_VIDEO_COMMANDS];
/* Worst case remains bounded by the old 21 words/quad. Pack four opcodes
 * together across vertices and retain unchanged color/texture/polygon state.
 * DMA owns this buffer until glCallList returns; no END (libnds restriction). */
static uint32_t list[1+21*X4_VIDEO_COMMANDS] __attribute__((aligned(32)));
static unsigned used,last_words;
/* The CPU owns the list until prepare finishes. Assemble opcode bytes in
 * a local word and commit once per four commands, rather than repeatedly
 * reading/modifying the list in RAM. Parameter order and list size stay exact. */
typedef struct {uint32_t* next;uint32_t* op;unsigned lane,word;} FifoWriter;
static uint16_t upload_slots[X4_VIDEO_TILES],palette_slots[X4_VIDEO_GPU_PALETTES];
static unsigned prepared,prepared_clear,prepared_uploads,prepared_palettes;
static X4GpuTransferStats transfer_stats;
static unsigned total_underflows,total_overflows,total_late,total_waits;
X4GpuTransferStats x4_gpu_transfer_stats(void){return transfer_stats;}
/* The 3D rasterizer restarts at line 214, within display VBlank. Small
 * interrupt-masked transfers restore texture mapping well before that.
 * Unfinished uploads retain the previous image until another safe window. */
#define UPLOAD_LAST_START 204
#define UPLOAD_CHUNK_TILES 32 /* 8 KiB of indexed pixels, bounded even for a full bank. */
static void texture_bank(unsigned bank,int lcd){
 switch(bank){
  case 0:vramSetBankA(lcd?VRAM_A_LCD:VRAM_A_TEXTURE_SLOT0);break;
  case 1:vramSetBankB(lcd?VRAM_B_LCD:VRAM_B_TEXTURE_SLOT1);break;
  case 2:vramSetBankC(lcd?VRAM_C_LCD:VRAM_C_TEXTURE_SLOT2);break;
  case 3:vramSetBankD(lcd?VRAM_D_LCD:VRAM_D_TEXTURE_SLOT3);break;
  case 4:vramSetBankE(lcd?VRAM_E_LCD:VRAM_E_TEX_PALETTE);break;
  case 5:vramSetBankF(lcd?VRAM_F_LCD:VRAM_F_TEX_PALETTE_SLOT4);break;
  default:vramSetBankG(lcd?VRAM_G_LCD:VRAM_G_TEX_PALETTE_SLOT5);break;
 }
}
static unsigned upload_batches,uploaded_tiles;
unsigned x4_gpu_upload_batches(void){return upload_batches;}
unsigned x4_gpu_upload_tiles(void){return uploaded_tiles;}
static inline __attribute__((always_inline)) void emit(FifoWriter* out,unsigned opcode,uint32_t a,uint32_t b,unsigned parameters){
 if(out->lane==4){if(out->op)*out->op=out->word;out->op=out->next++;out->word=0;out->lane=0;}
 out->word|=opcode<<(out->lane++*8);
 if(parameters)*out->next++=a;
 if(parameters==2)*out->next++=b;
}
static inline __attribute__((always_inline)) void emit_vertex(
 FifoWriter* out,unsigned uv,int x,int y,unsigned depth,int first){
 emit(out,FIFO_TEX_COORD,uv,0,1);
 emit(out,first?FIFO_VERTEX16:FIFO_VERTEX_XY,
      (uint16_t)x|((uint32_t)(uint16_t)y<<16),depth,first?2:1);
}
unsigned x4_gpu_words(void){return last_words;}
/* Stable descending ordering-table order, using a bounded radix sort. */
static void sort_commands(const X4VideoCommand* commands,unsigned count){
 unsigned low[256]={0},high[256]={0};
 /* Signed descending key = order XOR 0x7fff. Carry the high radix
  * digit with the index so the second pass walks the scratch buffer
  * without fetching orders through the scattered command indices. */
 for(unsigned i=0;i<count;++i){unsigned key=(uint16_t)commands[i].order^0x7fff;++low[key&255];++high[key>>8];}
 unsigned l=0,h=0;for(unsigned i=0;i<256;++i){unsigned a=low[i],b=high[i];low[i]=l;high[i]=h;l+=a;h+=b;}
 for(unsigned i=0;i<count;++i){
  unsigned key=(uint16_t)commands[i].order^0x7fff;
  scratch[low[key&255]++]=((key&0xff00)<<8)|i;
 }
 for(unsigned i=0;i<count;++i){unsigned entry=scratch[i];sorted[high[entry>>16]++]=(uint16_t)entry;}
}
void x4_gpu_init(void){
 videoSetMode(MODE_0_3D);vramSetBankA(VRAM_A_TEXTURE_SLOT0);vramSetBankB(VRAM_B_TEXTURE_SLOT1);
 vramSetBankC(VRAM_C_TEXTURE_SLOT2);vramSetBankD(VRAM_D_TEXTURE_SLOT3);
 x4_video_set_indexed(1);
 glInit();
 vramSetBankE(VRAM_E_TEX_PALETTE);vramSetBankF(VRAM_F_TEX_PALETTE_SLOT4);vramSetBankG(VRAM_G_TEX_PALETTE_SLOT5);
 glEnable(GL_TEXTURE_2D|GL_BLEND);glDisable(GL_ANTIALIAS|GL_OUTLINE);
 glViewport(0,0,255,191);glClearDepth(GL_MAX_DEPTH);glClearPolyID(63);
 glMatrixMode(GL_PROJECTION);glLoadIdentity();glOrthof32(32,288,216,24,-4096,4096);
 glMatrixMode(GL_MODELVIEW);glLoadIdentity();glMatrixMode(GL_TEXTURE);glLoadIdentity();
 x4_video_invalidate();x4_video_viewport(32,24,256,192);x4_video_enabled=1;
}
void x4_gpu_prepare(void){
 /* CPU-only work runs before waiting for VBlank. No hardware or texture
  * ownership changes here; the preceding GPU frame may still be rendering. */
 unsigned gain=255-x4_video_alpha(),clear=x4_video_clear();
 uint8_t fade[32];
 if(gain!=255)for(unsigned i=0;i<32;++i)fade[i]=i*gain/255;
 prepared_clear=gain==255?(clear&0x7fff):RGB15(fade[clear&31],fade[(clear>>5)&31],fade[(clear>>10)&31]);
 /* All four vertex colors of a textured command are white. */
 unsigned tile_color=gain==255?32767:RGB15(fade[31],fade[31],fade[31]);
 unsigned count;const X4VideoCommand* commands=x4_video_commands(&count);
 sort_commands(commands,count);
 FifoWriter out={.next=list+1,.lane=4};
 unsigned last_texture=~0u,last_palette=~0u,last_poly=~0u,last_color=~0u;
 unsigned last_raw_color=~0u;
 for(unsigned i=0;i<count;++i){
  const X4VideoCommand* c=&commands[sorted[i]];unsigned slot=c->tile;
  unsigned texture=c->textured?(slot*32)|(1<<20)|(1<<23)|(GL_RGB256<<26)|((c->textured==2)?1u<<29:0):0;
  unsigned poly=POLY_ALPHA(c->semi?16:31)|POLY_CULL_NONE|POLY_ID(c->semi?(i%62)+1:0);
  if(texture!=last_texture){emit(&out,FIFO_TEX_FORMAT,texture,0,1);last_texture=texture;}
  if(c->textured&&c->palette!=last_palette){emit(&out,FIFO_PAL_FORMAT,c->palette*32,0,1);last_palette=c->palette;}
  if(poly!=last_poly){emit(&out,FIFO_POLY_FORMAT,poly,0,1);last_poly=poly;}
  emit(&out,FIFO_BEGIN,GL_QUADS,0,1);
  /* UV endpoints occupy disjoint fields. Toggling an endpoint is an XOR,
   * avoiding coordinate arrays and four-element flip loops per quad. */
  unsigned u=c->w<<4,v=c->h<<20;
  unsigned uv=((c->flip&1)?u:0)|((c->flip&2)?v:0);
  if(c->textured){
   /* x4_video_tile emits a white rectangle. Reuse each edge for its
    * two corners, including the coordinate transform for HUD tiles. */
   if(tile_color!=last_color){emit(&out,FIFO_COLOR,tile_color,0,1);last_color=tile_color;}
   last_raw_color=32767;
   int left=c->x[0],right=c->x[1],top=c->y[0],bottom=c->y[2];
   if(c->screen_space){
    left=32+left*4/5;right=32+right*4/5;
    top=24+top*4/5;bottom=24+bottom*4/5;
   }
   emit_vertex(&out,uv,left,top,i,1);
   emit_vertex(&out,uv^u,right,top,i,0);
   emit_vertex(&out,uv^u^v,right,bottom,i,0);
   emit_vertex(&out,uv^v,left,bottom,i,0);
   continue;
  }
  for(unsigned j=0;j<4;++j){unsigned color=c->color[j];
   if(gain!=255){
    if(color==last_raw_color)color=last_color;
    else{
     last_raw_color=color;
     color=RGB15(fade[color&31],fade[(color>>5)&31],fade[(color>>10)&31]);
    }
   }
   int x=c->x[j],y=c->y[j];if(c->screen_space){x=32+x*4/5;y=24+y*4/5;}
   if(color!=last_color){emit(&out,FIFO_COLOR,color,0,1);last_color=color;}
   emit_vertex(&out,uv^((j==1||j==2)?u:0)^((j>=2)?v:0),x,y,i,j==0);
  }
 }
 /* Commit the final group even when it has fewer than four opcodes. */
 if(out.op)*out.op=out.word;
 used=(unsigned)(out.next-list);last_words=used-1;
 /* Flush RAM before VBlank while all texture banks remain available.
  * This plan is read-only: repeated preparation must not lose dirty tiles. */
 prepared_uploads=x4_video_dirty_tiles(upload_slots);
 for(unsigned j=0;j<prepared_uploads;){unsigned first=upload_slots[j++],tiles=1;
  while(j<prepared_uploads&&upload_slots[j]==first+tiles&&(first+tiles)/512==first/512){++j;++tiles;}
  DC_FlushRange(x4_video_peek_indexed(first),tiles*256);
 }
 prepared_palettes=x4_video_dirty_palettes(palette_slots);
 for(unsigned i=0;i<prepared_palettes;++i)DC_FlushRange(x4_video_peek_palette(palette_slots[i]),512);
 prepared=1;
}
static void safe_copy(unsigned bank,const void* data,void* target,unsigned bytes){
 for(;;){int irq_state=enterCriticalSection();unsigned line=REG_VCOUNT;
  if(line>=192&&line<=UPLOAD_LAST_START){
   if(line>transfer_stats.start_max)transfer_stats.start_max=line;
   texture_bank(bank,1);dmaCopyWords(3,data,target,bytes);texture_bank(bank,0);
   unsigned restored=REG_VCOUNT;
   if(restored>transfer_stats.restore_max)transfer_stats.restore_max=restored;
   if(restored<192||restored>=214)++transfer_stats.late;
   leaveCriticalSection(irq_state);return;
  }
  leaveCriticalSection(irq_state);++transfer_stats.waits;swiWaitForVBlank();
 }
}
void x4_gpu_present(void){
 if(!prepared)x4_gpu_prepare(); /* Small frontend frames retain this shortcut. */
 upload_batches=uploaded_tiles=0;memset(&transfer_stats,0,sizeof transfer_stats);
 transfer_stats.status=GFX_CONTROL&((1u<<12)|(1u<<13));
 total_underflows+=!!(transfer_stats.status&(1u<<12));total_overflows+=!!(transfer_stats.status&(1u<<13));
 glEnable(GL_COLOR_UNDERFLOW|GL_POLY_OVERFLOW); /* acknowledge sticky flags */
 for(unsigned i=0;i<prepared_palettes;++i){unsigned slot=palette_slots[i];
  unsigned bank=slot<128?4:slot<160?5:6,local=slot<128?slot:slot<160?slot-128:slot-160;
  uint8_t* target=bank==4?(uint8_t*)VRAM_E:bank==5?(uint8_t*)VRAM_F:(uint8_t*)VRAM_G;
  safe_copy(bank,x4_video_peek_palette(slot),target+local*512,512);int dirty;x4_video_palette(slot,&dirty);
 }
 for(unsigned j=0;j<prepared_uploads;){unsigned first=upload_slots[j],tiles=1;
  while(j+tiles<prepared_uploads&&tiles<UPLOAD_CHUNK_TILES&&upload_slots[j+tiles]==first+tiles&&(first+tiles)/512==first/512)++tiles;
  unsigned bank=first/512,offset=(first%512)*256;
  uint8_t* target=bank==0?(uint8_t*)VRAM_A:bank==1?(uint8_t*)VRAM_B:bank==2?(uint8_t*)VRAM_C:(uint8_t*)VRAM_D;
  safe_copy(bank,x4_video_peek_indexed(first),target+offset,tiles*256);
  /* Consume only after physical transfer. All waits leave every bank mapped
   * to textures, and neither frame advances the cache during this loop. */
  for(unsigned k=0;k<tiles;++k){int dirty;x4_video_texture(first+k,&dirty);}
  ++upload_batches;uploaded_tiles+=tiles;j+=tiles;
 }
 total_late+=transfer_stats.late;total_waits+=transfer_stats.waits;
 transfer_stats.underflows=total_underflows;transfer_stats.overflows=total_overflows;transfer_stats.total_late=total_late;transfer_stats.total_waits=total_waits;
 glClearColor(prepared_clear&31,(prepared_clear>>5)&31,(prepared_clear>>10)&31,31);
 glMatrixMode(GL_MODELVIEW);glLoadIdentity();
 if(used>1){list[0]=last_words;glCallList(list);}
 glFlush(GL_TRANS_MANUALSORT);
 prepared=0;
}
