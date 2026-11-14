#include <nds.h>
#include "x4_video.h"
#include <assert.h>
#include <string.h>
#include <stdio.h>
uint8_t mock_vram[524288];uint32_t GFX_TEX_FORMAT;
static int banks[4],projection[6],verts,flushes,copies;static unsigned current_tex;
static int vertex_x[X4_VIDEO_COMMANDS*4],vertex_y[X4_VIDEO_COMMANDS*4],vertex_z[X4_VIDEO_COMMANDS*4];
static int current_u,current_v,vertex_u[X4_VIDEO_COMMANDS*4],vertex_v[X4_VIDEO_COMMANDS*4];
static unsigned current_color,vertex_color[X4_VIDEO_COMMANDS*4];
static unsigned hardware_events,bank_writes;
static size_t copied_bytes;
static int protected_slot=-1;
void videoSetMode(int v){assert(v==MODE_0_3D);}
void vramSetBankA(int v){banks[0]=v;++hardware_events;++bank_writes;}void vramSetBankB(int v){banks[1]=v;++hardware_events;++bank_writes;}void vramSetBankC(int v){banks[2]=v;++hardware_events;++bank_writes;}void vramSetBankD(int v){banks[3]=v;++hardware_events;++bank_writes;}
void glInit(void){}void glEnable(int v){(void)v;}void glDisable(int v){(void)v;}
void glViewport(int a,int b,int c,int d){assert(a==0&&b==0&&c==255&&d==191);}
void glClearDepth(int v){assert(v==32767);}void glClearPolyID(int v){assert(v==63);}
void glMatrixMode(int v){(void)v;++hardware_events;}void glLoadIdentity(void){++hardware_events;}
void glOrthof32(int a,int b,int c,int d,int e,int f){projection[0]=a;projection[1]=b;projection[2]=c;projection[3]=d;projection[4]=e;projection[5]=f;}
void glClearColor(int a,int b,int c,int d){assert(a<=31&&b<=31&&c<=31&&d==31);++hardware_events;}
void glPolyFmt(unsigned v){assert(((v>>16)&31)>0);}
void glBegin(int v){assert(v==GL_QUADS);current_tex=GFX_TEX_FORMAT;}
void glColor(unsigned v){assert(v<=32767);current_color=v;}void glTexCoord2t16(int u,int v){assert(u>=0&&u<=256&&v>=0&&v<=256);current_u=u;current_v=v;}
void glVertex3v16(int x,int y,int z){(void)z;if(current_tex){assert(((current_tex>>26)&7)==GL_RGBA);assert(((current_tex>>20)&7)==1&&((current_tex>>23)&7)==1);}
 assert(verts<X4_VIDEO_COMMANDS*4);vertex_color[verts]=current_color;vertex_x[verts]=x;vertex_y[verts]=y;vertex_z[verts]=z;vertex_u[verts]=current_u;vertex_v[verts++]=current_v;
}
void glEnd(void){}void glFlush(int v){assert(v==GL_TRANS_MANUALSORT);++flushes;++hardware_events;}
void glCallList(const uint32_t* list){
 ++hardware_events;
 assert(!((uintptr_t)list&31));unsigned end=list[0]+1,pos=1;int z=0;
 while(pos<end){uint32_t commands=list[pos++];for(unsigned j=0;j<4;++j){unsigned op=(commands>>(j*8))&255;
  if(!op)continue;assert(pos<end);uint32_t p=list[pos++];
  switch(op){
   case FIFO_TEX_FORMAT:GFX_TEX_FORMAT=p;break;
   case FIFO_POLY_FORMAT:glPolyFmt(p);break;
   case FIFO_BEGIN:glBegin(p);break;
   case FIFO_COLOR:glColor(p);break;
   case FIFO_TEX_COORD:glTexCoord2t16((int16_t)p,(int16_t)(p>>16));break;
   case FIFO_VERTEX16:assert(pos<end);z=(int16_t)list[pos++];/* fall through */
   case FIFO_VERTEX_XY:glVertex3v16((int16_t)p,(int16_t)(p>>16),z);break;
   default:assert(!"invalid GPU command");
  }
 }}assert(pos==end);
}
void DC_FlushRange(const void* p,size_t n){assert(!((uintptr_t)p&31)&&n&&!(n%512)&&n<=131072);}
void dmaCopyWords(int channel,const void* p,void* q,size_t n){assert(channel==3&&n&&!(n%512)&&n<=131072);assert((uint8_t*)q>=mock_vram&&(uint8_t*)q+n<=mock_vram+sizeof mock_vram);assert(((size_t)((uint8_t*)q-mock_vram)%131072)+n<=131072);if(protected_slot>=0){uint8_t* clean=mock_vram+protected_slot*512;assert((uint8_t*)q+n<=clean||(uint8_t*)q>=clean+512);}memcpy(q,p,n);++copies;copied_bytes+=n;}
void x4_gpu_init(void);void x4_gpu_prepare(void);void x4_gpu_present(void);unsigned x4_gpu_words(void);
unsigned x4_gpu_upload_batches(void);unsigned x4_gpu_upload_tiles(void);
int main(void){
 uint8_t raw[128]={0x21};uint16_t pal[16]={0};pal[1]=0x001f;pal[2]=0x03e0;
 x4_gpu_init();assert(projection[0]==32&&projection[1]==288&&projection[2]==216&&projection[3]==24);
 x4_video_begin(0);assert(!x4_video_tile(raw,8,4,pal,16,32,24,16,16,0,0,1));
 unsigned count;const X4VideoCommand* c=x4_video_commands(&count);unsigned oldslot=c[0].tile;
 unsigned events=hardware_events;x4_gpu_prepare();assert(hardware_events==events&&!copies&&!verts);
 x4_gpu_present();assert(copies==1&&verts==4&&flushes==1);
 assert(vertex_x[0]==32&&vertex_x[1]==48&&vertex_y[0]==24&&vertex_y[2]==40);
 assert(((uint16_t*)mock_vram)[oldslot*256]==0x801f&&((uint16_t*)mock_vram)[oldslot*256+1]==0x83e0);
 assert(banks[0]==1&&banks[1]==2&&banks[2]==3&&banks[3]==4);
 pal[1]=0x7c00;x4_video_begin(0);verts=copies=0;
 x4_video_screen_space(1);assert(!x4_video_tile(raw,8,4,pal,16,0,0,16,16,0,0,1));
 unsigned newslot=c[0].tile;assert(newslot!=oldslot);x4_gpu_present();assert(copies==1);
 assert(((uint16_t*)mock_vram)[oldslot*256]==0x801f&&((uint16_t*)mock_vram)[newslot*256]==0xfc00);
 assert(vertex_x[0]==32&&vertex_y[0]==24&&vertex_x[1]==44&&vertex_y[2]==36);
 x4_video_begin(0);verts=0;
 x4_video_order(-2);assert(!x4_video_tile(raw,8,4,pal,16,32,24,16,16,3,1,1));
 int x[4]={40,48,48,40},y[4]={32,32,40,40};uint16_t colors[4]={1,2,3,4};
 x4_video_order(3);assert(!x4_video_quad(x,y,colors,0));x4_gpu_present();
 assert(verts==8&&vertex_x[0]==40&&vertex_x[4]==32);
 for(unsigned j=0;j<4;++j){assert(vertex_color[j]==colors[j]);assert(vertex_color[j+4]==32767);}
 for(unsigned i=0;i<8;++i)assert(vertex_z[i]==(int)(i/4));
 assert(vertex_u[4]==256&&vertex_v[4]==256&&vertex_u[6]==0&&vertex_v[6]==0);
 x4_video_begin(0);verts=0;
 for(unsigned i=0;i<X4_VIDEO_COMMANDS;++i)assert(!x4_video_tile(raw,8,4,pal,16,32,24,16,16,0,0,1));
 x4_gpu_present();assert(verts==X4_VIDEO_COMMANDS*4);
 unsigned compact_words=x4_gpu_words();assert(compact_words<21*X4_VIDEO_COMMANDS*3/4);
 for(unsigned i=0;i<(unsigned)verts;++i)assert(vertex_z[i]==(int)(i/4));
 printf("PASS actual N06.4 GPU: 1500 uniform quads %u words versus %u; depth, flip, ordering, RGB555 and 1:1 projection\n",compact_words,21*X4_VIDEO_COMMANDS);
 x4_video_begin(0);verts=0;
 for(unsigned i=0;i<X4_VIDEO_COMMANDS;++i)assert(!x4_video_quad(x,y,colors,0));
 x4_gpu_present();assert(verts==X4_VIDEO_COMMANDS*4&&x4_gpu_words()<=21*X4_VIDEO_COMMANDS);
 for(unsigned i=0;i<(unsigned)verts;++i)assert(vertex_color[i]==colors[i%4]);
 x4_video_begin(0);verts=copies=0;unsigned banks_before=bank_writes;
 assert(!x4_video_tile(raw,8,4,pal,16,32,24,16,16,0,0,1));
 events=hardware_events;x4_gpu_prepare();assert(hardware_events==events&&!copies&&!verts);
 x4_gpu_present();assert(!copies&&bank_writes==banks_before&&verts==4);
 puts("PASS CPU preparation touches no GPU/VRAM; cached frame uploads/remaps no texture banks");
 assert(!x4_gpu_upload_batches()&&!x4_gpu_upload_tiles());
 /* Adjacent uploads must be byte-identical while using one transfer per
  * run, with a mandatory split at every 128 KiB texture-bank boundary. */
 for(unsigned total=300;total<=1024;total+=724){
  x4_video_begin(0);x4_video_begin(0);verts=copies=0;copied_bytes=0;
  for(unsigned i=0;i<total;++i)assert(!x4_video_tile(raw,8,4,pal,16,32,24,16,16,0,0,50000+total+i));
  uint16_t slots[X4_VIDEO_TILES];unsigned pending=x4_video_dirty_tiles(slots),runs=0;
  assert(pending==total);
  for(unsigned i=0;i<pending;++i)if(!i||slots[i]!=slots[i-1]+1||slots[i]/256!=slots[i-1]/256)++runs;
  x4_gpu_prepare();x4_gpu_present();
  assert(copies==(int)runs&&copies<=4&&copied_bytes==total*512);
  assert(x4_gpu_upload_batches()==runs&&x4_gpu_upload_tiles()==total);
  assert(x4_video_stats().uploaded_bytes==total*512&&!x4_video_dirty_tiles(slots));
  x4_video_commands(&count);assert(count==total);
  for(unsigned i=0;i<count;++i){int dirty;const uint16_t* pixels=x4_video_rgba_texture(c[i].tile,&dirty);assert(!dirty);assert(!memcmp(pixels,mock_vram+c[i].tile*512,512));}
  printf("PASS %u dirty tiles: %u DMA batches, %zu unchanged bytes, bank boundaries protected\n",total,runs,copied_bytes);
 }
 x4_video_begin(0);x4_video_begin(0);verts=copies=0;copied_bytes=0;
 for(unsigned i=0;i<8;++i)assert(!x4_video_tile(raw,8,4,pal,16,32,24,16,16,0,0,70000+i));
 uint16_t slots[X4_VIDEO_TILES];assert(x4_video_dirty_tiles(slots)==8);
 protected_slot=slots[3];int dirty;const uint16_t* ready=x4_video_rgba_texture(protected_slot,&dirty);assert(dirty);
 memcpy(mock_vram+protected_slot*512,ready,512); /* Already uploaded individually. */
 unsigned pending=x4_video_dirty_tiles(slots),runs=0;assert(pending==7);
 for(unsigned i=0;i<pending;++i)if(!i||slots[i]!=slots[i-1]+1||slots[i]/256!=slots[i-1]/256)++runs;
 x4_gpu_prepare();x4_gpu_present();assert(copies==(int)runs&&copied_bytes==7*512&&runs>=2);
 x4_video_commands(&count);assert(count==8);
 for(unsigned i=0;i<count;++i){const uint16_t* pixels=x4_video_rgba_texture(c[i].tile,&dirty);assert(!memcmp(pixels,mock_vram+c[i].tile*512,512));}
 puts("PASS batched DMA never spans the clean slot between two dirty runs");
}
