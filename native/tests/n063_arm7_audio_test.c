#include <nds.h>
#include <calico/nds/arm9/sound.h>
#include "x4_arm7_audio.h"
#include "x4_sfx.h"
#include "x4_audio.h"
#include "x4_n05_platform.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static unsigned locked,active_mask,pending_stop,starts,syncs,volume[16],panning[16];
static int automatic;
static unsigned pcm_publications;
static const int16_t* playing[16];
void mmLockChannels(unsigned mask){assert(mask==0xff00);locked=mask;}
void soundSetAutoUpdate(bool enabled){automatic=enabled;}
void DC_FlushRange(const void* data,size_t bytes){assert(!((uintptr_t)data&31)&&!(bytes&31));++pcm_publications;}
void soundPreparePcm(unsigned channel,unsigned vol,unsigned pan,unsigned timer,SoundMode mode,SoundFmt fmt,const void* data,unsigned point,unsigned words){
 assert(channel&SOUND_START);channel&=15;
 assert(channel>=8&&(locked&(1u<<channel))&&pan<=127&&timer==soundTimerFromHz(18900));
 assert(fmt==SoundFmt_Pcm16&&!point);
 assert(!((uintptr_t)data&31)&&!pending_stop);playing[channel]=data;assert(playing[channel][0]>=1000);
 if(mode==SoundMode_Repeat){
  if(playing[channel][0]==1800){
   assert(words==4);
   for(unsigned i=0;i<8;++i)assert(playing[channel][i]==1800);
  }else{
   assert(words==5);
   for(unsigned i=0;i<10;++i)assert(playing[channel][i]==1100+(int)(i%5));
  }
 }else{
  assert(mode==SoundMode_OneShot&&words==4);
  for(unsigned i=5;i<8;++i)assert(playing[channel][i]==0);
 }
 panning[channel]=pan;volume[channel]=vol;active_mask|=1u<<channel;++starts;
}
void soundStop(unsigned mask){assert(!(mask&~0xff00));pending_stop|=mask;}
void soundSynchronize(void){
 /* Memory must remain readable until the ARM7 acknowledges STOP. */
 for(unsigned i=8;i<16;++i)if(pending_stop&(1u<<i)){if(playing[i])assert(playing[i][0]>=1000);playing[i]=NULL;}
 active_mask&=~pending_stop;pending_stop=0;++syncs;
}
unsigned soundGetActiveChannels(void){assert(automatic);return active_mask;}
void soundChSetVolume(unsigned ch,unsigned vol){assert(ch>=8&&ch<16);volume[ch]=vol;}
static void word(FILE* f,unsigned v){for(unsigned i=0;i<4;++i)assert(fputc((v>>(i*8))&255,f)!=EOF);}
static int music_request=-1,music_playing;
int x4_n05_music_request(int* selection,unsigned* gain){
 if(music_request<0)return 0;
 *selection=music_request;*gain=128;music_request=-1;return 1;
}
unsigned x4_n05_music_gain(void){return 128;}
void x4_n05_music_status(int playing){music_playing=playing;}

static void check_centered_music(const char* folder){
 char path[1024];snprintf(path,sizeof path,"%s/00.pcm",folder);
 FILE* f=fopen(path,"wb");assert(f);
 const int16_t pcm[]={1000,-2050,3400};
 assert(fwrite(pcm,sizeof pcm[0],3,f)==3&&!fclose(f));
 x4_audio_init(folder,folder);music_request=0;x4_audio_update();
 assert(music_playing&&x4_audio_track()==0);
 int16_t frames[9]={0};frames[8]=4321;x4_audio_read_stereo(frames,4);
 const int16_t expected[]={500,500,-1025,-1025,1700,1700,0,0,4321};
 assert(!memcmp(expected,frames,sizeof frames)&&!music_playing&&x4_audio_track()==-1&&!x4_audio_errors());
 x4_audio_close();
}

int main(int argc,char** argv){
 assert(argc==2);char path[1024],hidden[1024];snprintf(path,sizeof path,"%s/07_2.sfx",argv[1]);snprintf(hidden,sizeof hidden,"%s/hidden.sfx",argv[1]);
 FILE* f=fopen(path,"wb");assert(f);assert(fwrite("X4SFX01\0",1,8,f)==8);word(f,10);word(f,18900);
 for(unsigned i=0;i<10;++i){word(f,(i<<24)|0x80|(i==9?0x8000:0));word(f,i*16);word(f,i==8?1:5);word(f,i==1||i==8);}
 for(unsigned i=0;i<10;++i)for(unsigned j=0;j<8;++j){unsigned v=1000+i*100+j;fputc(v&255,f);fputc(v>>8,f);}assert(!fclose(f));
 x4_arm7_audio_init();x4_sfx_config(argv[1]);assert(!x4_sfx_bank(7,2));
 assert(!x4_sfx_play(2,0));assert(!rename(path,hidden));
 assert(!x4_sfx_play(2,0));/* A cache hit must work with the source file absent. */
 assert(!rename(hidden,path));assert(!x4_sfx_play(2,1));
 int16_t output[32];for(unsigned i=0;i<32;++i)output[i]=i-16;int16_t before[32];memcpy(before,output,sizeof output);
 x4_sfx_mix(output,32);assert(!memcmp(before,output,sizeof output));
 x4_sfx_mix_stereo(output,16);assert(!memcmp(before,output,sizeof output));
 X4SfxStats s=x4_sfx_stats();assert(s.hits==1&&s.misses==2&&s.active==2&&s.starts==3&&s.bytes==64);
 assert(pcm_publications==2); /* Replaying cached PCM needs no new flush. */
 x4_arm7_audio_mute(1);for(unsigned i=8;i<16;++i)assert(!volume[i]);
 x4_arm7_audio_mute(0);for(unsigned i=8;i<16;++i)assert(volume[i]==0x7f0);
 active_mask&=~(1u<<8);assert(x4_sfx_finished(2,0)&&!x4_sfx_finished(2,1));
 assert(!x4_sfx_bank(7,2));assert(!active_mask&&!x4_sfx_stats().bytes);
 for(unsigned i=0;i<8;++i)assert(!x4_sfx_play(2,i));assert(x4_sfx_play(2,8)==-7);
 x4_sfx_stop(2,3);assert(!x4_sfx_play(2,8));
 x4_sfx_stop(255,0);
 assert(!x4_sfx_play_at(2,0,5,1));assert(panning[8]==30&&volume[8]==0x7f0*84/128);
 /* Reuse identical cached PCM at the opposite side, with no new flush. */
 unsigned published=pcm_publications;
 assert(!x4_sfx_play_at(2,0,27,1));assert(panning[8]==97&&volume[8]==0x7f0*84/128);
 assert(pcm_publications==published);
 x4_arm7_audio_mute(1);assert(!volume[8]);x4_arm7_audio_mute(0);assert(volume[8]==0x7f0*84/128);
 assert(!x4_sfx_play_at(2,0,27,0));assert(panning[8]==64&&volume[8]==0x7f0);
 assert(!x4_sfx_play_at(2,0,-1,1));assert(panning[8]==64&&volume[8]==0x7f0);
 assert(!x4_sfx_play_at(2,0,1000,1));assert(panning[8]==97);
 assert(!x4_sfx_play_at(2,0,16,1));assert(panning[8]==64&&volume[8]==0x7f0);
 x4_sfx_stop(255,0);assert(!x4_sfx_play_at(2,9,5,1));assert(panning[8]==64&&volume[8]==0x7f0);
 x4_sfx_close();assert(!active_mask&&!pending_stop&&syncs>=starts&&!x4_sfx_errors());
 /* One cached hardware voice and one streamed loop must coexist when the
  * DS PCM budget is full. Stop must release each transport independently. */
 x4_sfx_set_cache_limit(32);assert(!x4_sfx_bank(7,2));
 assert(!x4_sfx_play(2,0));assert(!x4_sfx_play(2,1));
 assert(x4_sfx_stats().bytes==32&&x4_sfx_stats().active==2);
 memset(output,0,sizeof output);x4_sfx_mix(output,12);
 for(unsigned i=0;i<12;++i)assert(output[i]==1100+(int)(i%5));
 x4_sfx_stop(2,1);assert(x4_sfx_finished(2,1)&&!x4_sfx_finished(2,0));
 x4_sfx_close();
 x4_sfx_set_cache_limit(0);assert(!x4_sfx_bank(7,2));
 assert(!x4_sfx_play_at(2,0,5,1));
 memset(output,0,sizeof output);x4_sfx_mix(output,5);
 for(unsigned i=0;i<5;++i)assert(output[i]==(int)((1000+i)*84/128));
 assert(!x4_sfx_play_at(2,0,5,0));
 memset(output,0,sizeof output);x4_sfx_mix(output,5);
 for(unsigned i=0;i<5;++i)assert(output[i]==1000+(int)i);
 int16_t stereo[2*131+1]={0};stereo[2*131]=2731;
 assert(!x4_sfx_play_at(2,1,27,1));x4_sfx_mix_stereo(stereo,131);
 for(unsigned i=0;i<131;++i){
  int sample=1100+(int)(i%5);
  assert(stereo[2*i]==(sample*31*84)/16384);
  assert(stereo[2*i+1]==(sample*97*84)/16384);
 }
 assert(stereo[2*131]==2731);x4_sfx_stop(2,1);
 assert(!x4_sfx_play_at(2,0,5,1));
 memset(stereo,0,sizeof stereo);x4_sfx_mix_stereo(stereo,5);
 assert(stereo[0]==1000*98*84/16384&&stereo[1]==1000*30*84/16384);
 assert(!x4_sfx_play_at(2,0,5,0));
 memset(stereo,0,sizeof stereo);x4_sfx_mix_stereo(stereo,5);
 assert(stereo[0]==500&&stereo[1]==500);
 x4_sfx_close();x4_sfx_set_backend(NULL);
 check_centered_music(argv[1]);
 puts("PASS ARM7 PCM backend: reserved channels, alignment, padded length, stop acknowledgement, cache without file I/O, eight voices, mute, replacement lifetime, spatial pan/gain, short PCM, streamed stereo and centered music");
}
