#include "x4_sfx.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static unsigned le32(const unsigned char* b){return b[0]|(unsigned)b[1]<<8|(unsigned)b[2]<<16|(unsigned)b[3]<<24;}
static unsigned playing[8],hardware_signal;
static int hw_play(unsigned v,const int16_t* pcm,size_t samples,int loop,unsigned pan,unsigned gain){(void)loop;assert(pan<=127&&gain<=128);assert(v<8&&!((uintptr_t)pcm&31));playing[v]=1;
 for(size_t i=0;i<samples&&i<4096;++i)if(pcm[i]){++hardware_signal;break;}return 0;}
static void hw_stop(unsigned v){assert(v<8);playing[v]=0;}
static int hw_active(unsigned v){assert(v<8);return playing[v];}
int main(int argc,char** argv){
 assert(argc==3||argc==4);int hardware=argc==4;
 if(hardware){static const X4SfxBackend backend={hw_play,hw_stop,hw_active};x4_sfx_set_backend(&backend);}
 x4_sfx_config(argv[1]);FILE* cases=fopen(argv[2],"r");assert(cases);
 unsigned resource,bank,commands=0,accepted=0,pending=0,invalid=0,empty=0,signal=0;
 while(fscanf(cases,"%u %u",&resource,&bank)==2){
  assert(!x4_sfx_bank(resource,bank));char path[2048];snprintf(path,sizeof path,"%s/%02x_%u.sfx",argv[1],resource,bank);
  FILE* f=fopen(path,"rb");assert(f);unsigned char h[16];assert(fread(h,1,16,f)==16);unsigned count=le32(h+8);
  for(unsigned c=0;c<count;++c){
   unsigned char raw[16];assert(fread(raw,1,16,f)==16);++commands;
   x4_sfx_stop(255,0);int code=x4_sfx_play(bank,c);
   if(code== -5){assert(!le32(raw));++empty;continue;}
   if(code==X4_SFX_MISSING_PAYLOAD){assert(!le32(raw+8));++pending;continue;}
   if(code==X4_SFX_INVALID_SEQUENCE){
    X4SfxStats stats=x4_sfx_stats();
    ++invalid;assert(!le32(raw+8)&&stats.invalid_sequences==invalid);
    assert(stats.invalid_resource==resource&&stats.invalid_bank==bank&&stats.invalid_index==c&&stats.invalid_command==le32(raw));
    continue;
   }
   assert(code==0);++accepted;int16_t pcm[4096]={0};x4_sfx_mix(pcm,4096);
   for(unsigned i=0;i<4096;++i)if(pcm[i]){++signal;break;}
   x4_sfx_stop(bank,c);assert(x4_sfx_finished(bank,c));assert(!x4_sfx_errors());
  }
  fclose(f);
  /* A failed replacement must not discard the currently loaded bank. */
  assert(x4_sfx_bank(255,bank)!=0);assert(x4_sfx_play(bank,256)<0);
 }
 if(hardware)signal=hardware_signal;
 fclose(cases);printf("PCM effects commands=%u accepted=%u pending=%u invalid_sep=%u empty=%u audible-prefix=%u backend=%s\n",commands,accepted,pending,invalid,empty,signal,hardware?"ARM7 PCM cache":"ARM9 reference");fflush(stdout);
 assert(accepted>1750&&signal>1500);x4_sfx_close();puts("PASS native PCM effects");
}
