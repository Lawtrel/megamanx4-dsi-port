/* ARM9 control of Calico's existing ARM7 sound driver. PCM stays in shared
 * main RAM. Channels 8..15 are reserved; Maxmod's mono music stream uses 0.
 * A synchronous stop acknowledgement always precedes cache reuse/free. */
#include <nds.h>
#include <maxmod9.h>
#include <calico/nds/arm9/sound.h>
#include "x4_sfx.h"
#include "x4_arm7_audio.h"
static int muted;
static int play(unsigned voice,const int16_t* pcm,size_t samples,int loop){
 if(voice>=8||!pcm||!samples||((uintptr_t)pcm&31))return -1;
 unsigned bytes=(samples*2+31)&~31u;
 DC_FlushRange(pcm,bytes);
 soundPreparePcm((8+voice)|SOUND_START,muted?0:0x7f0,64,soundTimerFromHz(18900),
  loop?SoundMode_Repeat:SoundMode_OneShot,SoundFmt_Pcm16,pcm,0,(samples+1)/2);
 soundSynchronize();return 0;
}
static void stop(unsigned voice){if(voice<8){soundStop(1u<<(8+voice));soundSynchronize();}}
static int active(unsigned voice){return voice<8&&!!(soundGetActiveChannels()&(1u<<(8+voice)));}
void x4_arm7_audio_init(void){
 static const X4SfxBackend backend={play,stop,active};
 /* Maxmod uses acknowledged IPC for channel reservation. No competing owner
  * may program these eight channels while cached PCM is playing. */
 mmLockChannels(0xff00);soundSetAutoUpdate(true);x4_sfx_set_backend(&backend);
}
void x4_arm7_audio_mute(int value){muted=!!value;for(unsigned i=8;i<16;++i)soundChSetVolume(i,muted?0:0x7f0);}
