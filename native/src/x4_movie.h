#ifndef X4_MOVIE_H
#define X4_MOVIE_H
#include <stddef.h>
#include <stdint.h>
#define X4_MOVIE_PIXELS (256*192)
int x4_movie_decode(const uint8_t* data,size_t bytes,uint8_t* output,size_t capacity);
void x4_movie_config(const char* root);
void x4_movie_close(void);
int x4_movie_start(unsigned id);
void x4_movie_tick(void);
void x4_movie_skip(void);
int x4_movie_active(void);
int x4_movie_error(void);
unsigned x4_movie_id(void);
unsigned x4_movie_frame(void);
unsigned x4_movie_generation(void);
const uint16_t* x4_movie_pixels(void);
void x4_movie_audio(int16_t* output,size_t samples);
#endif
