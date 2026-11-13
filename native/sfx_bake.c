/* HOST-ONLY offline sample instrument renderer. Never linked into the NDS.
 * Uses the upstream synthesizer solely while preparing ordinary PCM assets. */
#include <psyz.h>
#include "../referencias/mmx4-decomp/tools/psyz/decomp/src/libspu/libspu_private.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
static union SpuUnion registers;
union SpuUnion* _spu_RXX=&registers;
size_t Psyz_CdPullSamples(short* out,size_t n){(void)out;(void)n;return 0;}
void Psyz_RcntAdd(int n){(void)n;}
void psyz_log(unsigned level,const char* file,unsigned line,const char* func,const char* fmt,...){
 (void)level;(void)file;(void)line;(void)func;(void)fmt;
}
void bake_reset(const unsigned char* body,unsigned size){
 Psyz_SpuReset(0);Psyz_SpuMemWrite(0x1000,body,size);
 Psyz_SpuWrite(0x180,0x3fff);Psyz_SpuWrite(0x182,0x3fff);
 Psyz_SpuWrite(0x1aa,0xc000);
}
void bake_voice(unsigned voice,unsigned offset,unsigned pitch,unsigned adsr1,unsigned adsr2,unsigned volume){
 unsigned r=(voice%24)*16;
 Psyz_SpuWrite(r,volume);Psyz_SpuWrite(r+2,volume);
 Psyz_SpuWrite(r+4,pitch);Psyz_SpuWrite(r+6,(0x1000+offset)/8);
 Psyz_SpuWrite(r+8,adsr1);Psyz_SpuWrite(r+10,adsr2);
 Psyz_SpuWrite(voice<16?0x188:0x18a,1u<<(voice%16));
}
void bake_keyoff(unsigned voice){Psyz_SpuWrite(voice<16?0x18c:0x18e,1u<<(voice%16));}
unsigned bake_active(void){unsigned mask=0;for(unsigned i=0;i<24;++i)if(Psyz_SpuRead(i*16+12))mask|=1u<<i;return mask;}
void bake_pull(short* out,unsigned n){Psyz_SpuPullSamples(out,n);}
