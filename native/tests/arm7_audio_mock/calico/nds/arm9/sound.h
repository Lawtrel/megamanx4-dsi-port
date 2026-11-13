#pragma once
#include <nds.h>
#define SOUND_START 16
typedef enum {SoundMode_Repeat=1,SoundMode_OneShot=2} SoundMode;
typedef enum {SoundFmt_Pcm16=1} SoundFmt;
static inline unsigned soundTimerFromHz(unsigned hz){return (33513982u/2+hz/2)/hz;}
void soundPreparePcm(unsigned,unsigned,unsigned,unsigned,SoundMode,SoundFmt,const void*,unsigned,unsigned);
void soundSynchronize(void);
void soundStop(unsigned);
unsigned soundGetActiveChannels(void);
void soundSetAutoUpdate(bool);
void soundChSetVolume(unsigned,unsigned);
