#ifndef X4_SFX_H
#define X4_SFX_H
#include <stdint.h>
#include <stddef.h>
void x4_sfx_config(const char* root);
void x4_sfx_character(unsigned character);
void x4_sfx_close(void);
int x4_sfx_bank(unsigned resource,unsigned bank);
int x4_sfx_play(unsigned bank,unsigned index);
void x4_sfx_stop(unsigned bank,unsigned index);
int x4_sfx_finished(unsigned bank,unsigned index);
void x4_sfx_mix(int16_t* out,size_t frames);
unsigned x4_sfx_errors(void);
#endif
