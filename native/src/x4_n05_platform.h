#ifndef X4_N05_PLATFORM_H
#define X4_N05_PLATFORM_H
/* Primitive ABI shared between game headers and the DSi SDK. */
void x4_n05_platform_init(const char* content_root);
void x4_n05_platform_close(void);
int x4_n05_platform_error(void);
unsigned x4_n05_missing_sfx(void);
/* Poll consumes the latest music request; -1 means stop, >=0 selection. */
int x4_n05_music_request(int* selection,unsigned* volume);
unsigned x4_n05_music_gain(void);
void x4_n05_music_status(int playing);
#endif
