#include "x4_lower_ui.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
static void preview(const char* root,const char* name,const unsigned char* data){
 char path[1024];snprintf(path,sizeof path,"%s/%s.ppm",root,name);FILE* f=fopen(path,"wb");assert(f);fprintf(f,"P6\n256 192\n255\n");
 const unsigned short* palette=x4_lower_palette();
 for(unsigned y=0;y<192;++y)for(unsigned x=0;x<256;++x){unsigned p=data[((y/8)*32+x/8)*32+(y&7)*4+(x&7)/2]>>((x&1)*4)&15,c=palette[p];unsigned char rgb[]={((c&31)*255+15)/31,(((c>>5)&31)*255+15)/31,(((c>>10)&31)*255+15)/31};assert(fwrite(rgb,1,3,f)==3);}
 assert(!fclose(f));
}
int main(int argc,char** argv){
 assert(argc==3);unsigned char font[2048];FILE* f=fopen(argv[1],"rb");assert(f&&fread(font,1,2048,f)==2048);fclose(f);
 X4PlayerPanel p={.active=1,.max_hp=48,.hp=37,.lives=3,.weapon=4,.weapons=255,.armor=15,.upgrades=0xf0ff};
 for(unsigned i=0;i<9;++i)p.energy[i]=48-i*3;
 p.tanks[0]=32;p.tanks[1]=17;p.tanks[2]=4;
 X4Radar radar={.active=1,.player_x=256,.player_y=144,.marker_count=2,.markers={{128,96,1},{304,128,2}}};
 for(unsigned y=0;y<18;++y)for(unsigned x=0;x<32;++x)radar.cells[y][x]=y>=14||(x>=10&&x<16&&y==10);
 x4_lower_reset();assert(x4_lower_mode()==X4_LOWER_PANEL);
 X4PlayerPanel before=p;const unsigned char* pixels=x4_lower_render(&p,&radar,0,font);assert(!memcmp(&p,&before,sizeof p));preview(argv[2],"panel_x",pixels);
 assert(x4_lower_touch(4,25,&p)==0);assert(x4_lower_touch(100,60,&p)==4);
 p.weapons=0;assert(x4_lower_touch(100,60,&p)==-1);p.weapons=255;
 p.character=1;assert(x4_lower_touch(100,60,&p)==-1);preview(argv[2],"panel_zero",x4_lower_render(&p,&radar,0,font));
 assert(x4_lower_touch(255,191,&p)==-1&&x4_lower_mode()==X4_LOWER_MAP);
 preview(argv[2],"radar",x4_lower_render(&p,&radar,0,font));
 radar.marker_count=999;radar.player_x=-999;radar.player_y=999;radar.markers[0].x=-32768;radar.markers[0].y=32767;
 x4_lower_render(&p,&radar,1,font);radar.active=0;preview(argv[2],"inactive_map",x4_lower_render(&p,&radar,0,font));
 assert(x4_lower_touch(100,180,&p)==-1&&x4_lower_mode()==X4_LOWER_DEBUG);
 assert(x4_lower_touch(10,150,&p)==X4_LOWER_OPEN_TEST);
 assert(x4_lower_touch(100,150,&p)==X4_LOWER_SIGMA_HITKILL);
 assert(x4_lower_touch(200,150,&p)==X4_LOWER_CREDITS);
 assert(x4_lower_touch(200,165,&p)==-1);
 assert(x4_lower_touch(100,143,&p)==-1);
 assert(x4_lower_touch(256,180,&p)==-1&&x4_lower_mode()==X4_LOWER_DEBUG);
 assert(x4_lower_touch(10,180,&p)==-1&&x4_lower_mode()==X4_LOWER_PANEL);
 p.active=0;assert(x4_lower_touch(100,60,&p)==-1);preview(argv[2],"inactive_panel",x4_lower_render(&p,&radar,0,font));
 puts("PASS lower UI: touch tabs, acquired X weapons only, Zero details, inactive controls, snapshot unchanged, bounded radar/tile writes");
}
