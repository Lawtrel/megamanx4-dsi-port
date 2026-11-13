/* Host diagnostic image of the submitted DS commands, not an emulator test. */
#include "x4_video.h"
#include <stdio.h>
#include <stdlib.h>
static const X4VideoCommand* preview_commands;
static int preview_compare(const void* a,const void* b){
 unsigned i=*(const unsigned*)a,j=*(const unsigned*)b;
 int d=preview_commands[j].order-preview_commands[i].order;return d?d:(i>j)-(i<j);
}
static void n06_preview(const char* path){
 unsigned count;preview_commands=x4_video_commands(&count);unsigned order[X4_VIDEO_COMMANDS];
 int vx,vy,vw,vh;x4_video_get_viewport(&vx,&vy,&vw,&vh);
 static uint16_t pixels[256*192];for(unsigned i=0;i<256*192;++i)pixels[i]=x4_video_clear();
 for(unsigned i=0;i<count;++i)order[i]=i;qsort(order,count,sizeof *order,preview_compare);
 for(unsigned k=0;k<count;++k){const X4VideoCommand* c=&preview_commands[order[k]];int dirty;
  const uint16_t* tile=c->textured?x4_video_rgba_texture(c->tile,&dirty):NULL;
  int left=256,right=0,top=192,bottom=0;
  for(unsigned j=0;j<4;++j){int x=c->screen_space?c->x[j]*256/320:(c->x[j]-vx)*256/vw,y=c->screen_space?c->y[j]*192/240:(c->y[j]-vy)*192/vh;if(x-1<left)left=x-1;if(x+1>right)right=x+1;if(y-1<top)top=y-1;if(y+1>bottom)bottom=y+1;}
  if(left<0)left=0;if(right>256)right=256;if(top<0)top=0;if(bottom>192)bottom=192;
  for(int y=top;y<bottom;++y)for(int x=left;x<right;++x){unsigned color;
   if(c->textured){int sx=(int)((x+0.5)*(c->screen_space?320:vw)/256)+(c->screen_space?0:vx)-c->x[0],sy=(int)((y+0.5)*(c->screen_space?240:vh)/192)+(c->screen_space?0:vy)-c->y[0];if(sx<0||sy<0||sx>=c->w||sy>=c->h)continue;
    if(c->flip&1)sx=c->w-1-sx;if(c->flip&2)sy=c->h-1-sy;
    color=tile[sy*16+sx];if(!(color&0x8000))continue;color&=32767;
   }else{
    int inside=0;color=0;
    for(unsigned tri=0;tri<2;++tri){unsigned a=0,b=tri?2:1,d=tri?3:2;
     double px=(x+0.5)*(c->screen_space?320:vw)/256+(c->screen_space?0:vx),py=(y+0.5)*(c->screen_space?240:vh)/192+(c->screen_space?0:vy);
     double area=(c->x[b]-c->x[a])*(c->y[d]-c->y[a])-(c->y[b]-c->y[a])*(c->x[d]-c->x[a]);if(!area)continue;
     double wb=((px-c->x[a])*(c->y[d]-c->y[a])-(py-c->y[a])*(c->x[d]-c->x[a]))/area;
     double wd=((c->x[b]-c->x[a])*(py-c->y[a])-(c->y[b]-c->y[a])*(px-c->x[a]))/area,wa=1-wb-wd;
     if(wa<0||wb<0||wd<0)continue;
     for(unsigned channel=0;channel<3;++channel){unsigned shift=channel*5;
      color|=(unsigned)(wa*((c->color[a]>>shift)&31)+wb*((c->color[b]>>shift)&31)+wd*((c->color[d]>>shift)&31))<<shift;
     }inside=1;break;
    }if(!inside)continue;
   }
   if(c->semi)color=((color&0x7bde)>>1)+((pixels[y*256+x]&0x7bde)>>1);pixels[y*256+x]=color;
  }
 }
 FILE* f=fopen(path,"wb");assert(f);fprintf(f,"P6\n256 192\n255\n");unsigned gain=255-x4_video_alpha();
 for(unsigned i=0;i<256*192;++i)for(unsigned channel=0;channel<3;++channel){unsigned v=(pixels[i]>>(channel*5))&31;fputc((v*255/31)*gain/255,f);}
 fclose(f);
}
