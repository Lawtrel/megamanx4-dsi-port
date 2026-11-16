#ifndef X4_CONTROLS_H
#define X4_CONTROLS_H
/* DS keypad ABI, shared by the ARM9 frontend and real-engine host tests. */
enum {
 X4_NDS_A=1u<<0,X4_NDS_B=1u<<1,X4_NDS_SELECT=1u<<2,X4_NDS_START=1u<<3,
 X4_NDS_RIGHT=1u<<4,X4_NDS_LEFT=1u<<5,X4_NDS_UP=1u<<6,X4_NDS_DOWN=1u<<7,
 X4_NDS_R=1u<<8,X4_NDS_L=1u<<9,X4_NDS_X=1u<<10,X4_NDS_Y=1u<<11,
 X4_INPUT_GIGA=1u<<6
};
static inline unsigned x4_controls_buttons(unsigned keys){
 const unsigned leave=X4_NDS_L|X4_NDS_R|X4_NDS_SELECT;
 if((keys&leave)==leave)return 0;
 /* A complete frontend chord consumes its action buttons. SELECT in the
  * stage selector is handled before this gameplay mapping is called. */
 if((keys&(X4_NDS_SELECT|X4_NDS_X))==(X4_NDS_SELECT|X4_NDS_X))keys&=~(X4_NDS_SELECT|X4_NDS_X);
 unsigned b=0;if(keys&X4_NDS_RIGHT)b|=1;if(keys&X4_NDS_LEFT)b|=2;
 if(keys&X4_NDS_UP)b|=4;if(keys&X4_NDS_DOWN)b|=8;
 if(keys&X4_NDS_Y)b|=16;if(keys&X4_NDS_X)b|=32;
 if(keys&X4_NDS_SELECT)b|=X4_INPUT_GIGA;
 if(keys&X4_NDS_B)b|=128;if(keys&X4_NDS_A)b|=256;
 if(keys&X4_NDS_R)b|=512;if(keys&X4_NDS_L)b|=1024;
 if(keys&X4_NDS_START)b|=1u<<16;
 return b;
}
#endif
