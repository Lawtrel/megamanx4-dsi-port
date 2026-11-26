#ifndef X4_SFX_H
#define X4_SFX_H
#include <stdint.h>
#include <stddef.h>
/* PCM memory belongs to ARM9 and remains immutable until stop() acknowledges
 * the ARM7/hardware reader has stopped. No FILE or game pointer crosses CPUs. */
typedef struct {
 int (*play)(unsigned voice,const int16_t* pcm,size_t samples,int loop,unsigned pan,unsigned gain);
 void (*stop)(unsigned voice);
 int (*active)(unsigned voice);
 /* Publish immutable PCM to the other CPU once, before its first play. */
 void (*prepare)(const int16_t* pcm,size_t bytes);
} X4SfxBackend;
typedef struct {unsigned hits,misses,bytes,starts,active;} X4SfxStats;
void x4_sfx_set_cache_limit(size_t bytes);
void x4_sfx_set_backend(const X4SfxBackend* backend);
X4SfxStats x4_sfx_stats(void);
void x4_sfx_config(const char* root);
void x4_sfx_character(unsigned character);
void x4_sfx_close(void);
int x4_sfx_bank(unsigned resource,unsigned bank);
int x4_sfx_play(unsigned bank,unsigned index);
/* Original screen position in units of ten pixels; -1 centers the effect.
 * Stereo is per request so the original options menu remains authoritative. */
int x4_sfx_play_at(unsigned bank,unsigned index,int position,int stereo);
void x4_sfx_stop(unsigned bank,unsigned index);
int x4_sfx_finished(unsigned bank,unsigned index);
void x4_sfx_mix(int16_t* out,size_t frames);
void x4_sfx_mix_stereo(int16_t* interleaved,size_t frames);
unsigned x4_sfx_errors(void);
#endif
