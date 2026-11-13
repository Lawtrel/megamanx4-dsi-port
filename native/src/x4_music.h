#ifndef X4_MUSIC_H
#define X4_MUSIC_H
#include <stdint.h>
#include <stddef.h>
typedef struct {
 void* file;
 unsigned selection,gain,loop,finished,error;
 size_t samples,position;
} X4Music;
int x4_music_open(X4Music* music,const char* directory,unsigned selection,unsigned gain);
size_t x4_music_read(X4Music* music,int16_t* destination,size_t frames);
void x4_music_close(X4Music* music);
#endif
