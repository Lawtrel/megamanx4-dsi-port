#ifndef X4_AUDIO_H
#define X4_AUDIO_H
#include <stdint.h>
#include <stddef.h>
void x4_audio_init(const char* music_root,const char* sfx_root);
void x4_audio_close(void);
void x4_audio_update(void);
void x4_audio_read(int16_t* output,size_t frames);
/* Output points to 2 * frames samples, interleaved left/right. */
void x4_audio_read_stereo(int16_t* output,size_t frames);
unsigned x4_audio_errors(void);
int x4_audio_track(void);
#endif
