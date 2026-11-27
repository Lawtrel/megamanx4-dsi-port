/* HOST-ONLY offline sample instrument renderer. Never linked into the NDS.
 * Uses the upstream synthesizer solely while preparing ordinary PCM assets. */
#include <psyz.h>
#include <libspu.h>
#include "../referencias/mmx4-decomp/tools/psyz/decomp/src/libspu/libspu_private.h"
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
static union SpuUnion registers;
union SpuUnion* _spu_RXX=&registers;
size_t Psyz_CdPullSamples(short* out,size_t n){(void)out;(void)n;return 0;}
void Psyz_RcntAdd(int n){(void)n;}
void psyz_log(unsigned level,const char* file,unsigned line,const char* func,const char* fmt,...){
 (void)level;(void)file;(void)line;(void)func;(void)fmt;
}
unsigned bake_api_version(void){return 2;}
int bake_reset(const unsigned char* body,unsigned size,unsigned reverb_depth){
 const unsigned sample_address=0x1000;
 const unsigned mode=SPU_REV_MODE_STUDIO_A;
 const unsigned room_address=_spu_rev_startaddr[mode];
 const unsigned end=reverb_depth?room_address*8:512*1024;
 if(reverb_depth>127||size>end-sample_address)return 0;
 Psyz_SpuReset(0);
 if(size)Psyz_SpuMemWrite(sample_address,body,size);
 SPUW(main_vol.left,0x3fff);SPUW(main_vol.right,0x3fff);
 unsigned control=SPU_CTRL_MASK_MUTE_SPU|SPU_CTRL_MASK_SPU_ENABLE;
 if(reverb_depth){
  struct rev_param_entry preset;
  memcpy(&preset,_spu_rev_param+mode*sizeof(preset),sizeof(preset));
  _spu_setReverbAttr(&preset);
  SPUW(rev_work_addr,room_address);
  unsigned gain=reverb_depth*0x7fff/127;
  SPUW(rev_vol.left,gain);SPUW(rev_vol.right,gain);
  control|=SPU_CTRL_MASK_REVERB_MASTER_ENABLE;
 }
 SPUW(spucnt,control);
 return 1;
}
void bake_voice(unsigned voice,unsigned offset,unsigned pitch,unsigned adsr1,unsigned adsr2,unsigned volume,unsigned reverb){
 if(voice>=24)return;
 unsigned r=voice*16;
 Psyz_SpuWrite(r,volume);Psyz_SpuWrite(r+2,volume);
 Psyz_SpuWrite(r+4,pitch);Psyz_SpuWrite(r+6,(0x1000+offset)/8);
 Psyz_SpuWrite(r+8,adsr1);Psyz_SpuWrite(r+10,adsr2);
 unsigned send_register=offsetof(SPU_RXX,rev_mode)+2*(voice/16);
 unsigned voice_mask=1u<<(voice%16);
 unsigned sends=Psyz_SpuRead(send_register);
 Psyz_SpuWrite(send_register,(sends&~voice_mask)|(reverb?voice_mask:0));
 Psyz_SpuWrite(voice<16?0x188:0x18a,voice_mask);
}
void bake_keyoff(unsigned voice){Psyz_SpuWrite(voice<16?0x18c:0x18e,1u<<(voice%16));}
unsigned bake_active(void){unsigned mask=0;for(unsigned i=0;i<24;++i)if(Psyz_SpuRead(i*16+12))mask|=1u<<i;return mask;}
void bake_pull(short* out,unsigned n){Psyz_SpuPullSamples(out,n);}
