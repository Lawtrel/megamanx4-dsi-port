/* Actual GPU preparation workload; hardware calls are recorded after the
 * measured region. The before/after FIFO byte streams must hash identically. */
#include <nds.h>
#include "x4_video.h"
#include <string.h>
uint8_t mock_vram[524288];uint32_t GFX_TEX_FORMAT;
static uint32_t stream_hash;
void videoSetMode(int n){(void)n;}
void vramSetBankA(int n){(void)n;}void vramSetBankB(int n){(void)n;}
void vramSetBankC(int n){(void)n;}void vramSetBankD(int n){(void)n;}
void glInit(void){}void glEnable(int n){(void)n;}void glDisable(int n){(void)n;}
void glViewport(int a,int b,int c,int d){(void)a;(void)b;(void)c;(void)d;}
void glClearDepth(int n){(void)n;}void glClearPolyID(int n){(void)n;}
void glMatrixMode(int n){(void)n;}void glLoadIdentity(void){}
void glOrthof32(int a,int b,int c,int d,int e,int f){(void)a;(void)b;(void)c;(void)d;(void)e;(void)f;}
void glClearColor(int a,int b,int c,int d){(void)a;(void)b;(void)c;(void)d;}
void glFlush(int n){(void)n;}
void DC_FlushRange(const void* p,size_t n){(void)p;(void)n;}
void dmaCopyWords(int n,const void* p,void* q,size_t z){(void)n;memcpy(q,p,z);}
void glCallList(const uint32_t* p){stream_hash=2166136261u;for(unsigned i=0;i<=p[0];++i)stream_hash=(stream_hash^p[i])*16777619u;}
void x4_gpu_prepare(void);void x4_gpu_present(void);
__attribute__((noinline,noipa)) void benchmark_start(void){__asm__ volatile("":::"memory");}
__attribute__((noinline,noipa)) void benchmark_end(void){__asm__ volatile("":::"memory");}
uint32_t unit_entry(unsigned total,unsigned mode){
 static uint8_t raw[128]={0x21};static uint16_t palette[16]={0,31,992};
 x4_video_enabled=1;x4_video_viewport(32,24,256,192);x4_video_begin(0);
 for(unsigned i=0;i<total;++i){
  x4_video_order((i*17)%90-45);x4_video_screen_space(mode==1&&i%7==0);
  if(mode==3){int x[4]={40,56,56,40},y[4]={30,30,46,46};uint16_t colors[4]={31,992,31744,32767};x4_video_quad(x,y,colors,i%3==0);}
  else x4_video_tile(raw,8,4,palette,16,40,30,16,16,i&3,i%5==0,1);
 }
 if(mode==2)x4_video_fade(90);
 benchmark_start();x4_gpu_prepare();benchmark_end();
 x4_gpu_present();return stream_hash;
}
