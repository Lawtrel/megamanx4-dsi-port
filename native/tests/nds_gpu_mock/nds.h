/* Test recorder for the actual DS submission code, not a game runtime. */
#ifndef NDS_GPU_MOCK_H
#define NDS_GPU_MOCK_H
#include <stdint.h>
#include <stddef.h>
typedef uint16_t vu16;
extern uint8_t mock_vram[524288];extern uint32_t GFX_TEX_FORMAT;
#define VRAM_A (mock_vram)
#define VRAM_B (mock_vram+131072)
#define VRAM_C (mock_vram+262144)
#define VRAM_D (mock_vram+393216)
enum {MODE_0_3D=8,VRAM_A_LCD=0,VRAM_B_LCD=0,VRAM_C_LCD=0,VRAM_D_LCD=0,
 VRAM_A_TEXTURE_SLOT0=1,VRAM_B_TEXTURE_SLOT1=2,VRAM_C_TEXTURE_SLOT2=3,VRAM_D_TEXTURE_SLOT3=4,
 GL_TEXTURE_2D=1,GL_BLEND=2,GL_ANTIALIAS=4,GL_OUTLINE=8,GL_MAX_DEPTH=32767,
 GL_PROJECTION=0,GL_MODELVIEW=1,GL_TEXTURE=3,GL_QUADS=1,GL_RGBA=7,
 POLY_CULL_NONE=3<<6,GL_TRANS_MANUALSORT=1};
#define RGB15(r,g,b) ((r)|((g)<<5)|((b)<<10))
#define POLY_ALPHA(a) ((a)<<16)
#define POLY_ID(i) ((i)<<24)
#define FIFO_COMMAND_PACK(a,b,c,d) ((a)|((b)<<8)|((c)<<16)|((d)<<24))
enum {FIFO_TEX_FORMAT=0x2a,FIFO_POLY_FORMAT=0x29,FIFO_BEGIN=0x40,FIFO_COLOR=0x20,FIFO_TEX_COORD=0x22,FIFO_VERTEX16=0x23,FIFO_VERTEX_XY=0x25};
void glCallList(const uint32_t* list);
void videoSetMode(int mode);void vramSetBankA(int mode);void vramSetBankB(int mode);void vramSetBankC(int mode);void vramSetBankD(int mode);
void glInit(void);void glEnable(int v);void glDisable(int v);void glViewport(int,int,int,int);
void glClearDepth(int);void glClearPolyID(int);void glMatrixMode(int);void glLoadIdentity(void);
void glOrthof32(int,int,int,int,int,int);void glClearColor(int,int,int,int);void glPolyFmt(unsigned);
void glBegin(int);void glColor(unsigned);void glTexCoord2t16(int,int);void glVertex3v16(int,int,int);void glEnd(void);void glFlush(int);
void DC_FlushRange(const void*,size_t);void dmaCopyWords(int,const void*,void*,size_t);
#endif
