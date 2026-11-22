/* CPU-side indexed tiles for the independent lower 2D engine. */
#include "x4_lower_ui.h"
#include "x4_lower_icons.h"
#include <stdio.h>
#include <string.h>
static unsigned char tiles[X4_LOWER_TILE_BYTES] __attribute__((aligned(4)));
static unsigned mode,selected;
static int clip_y0,clip_y1=192;
static unsigned ink,dim,red,green,cyan,gold;
static unsigned nearest(unsigned r,unsigned g,unsigned b){
 unsigned best=0,distance=~0u;
 for(unsigned i=0;i<16;++i){unsigned c=x4_lower_icon_palette[i];int dr=(c&31)-r,dg=((c>>5)&31)-g,db=((c>>10)&31)-b;
  unsigned d=dr*dr+dg*dg+db*db;if(d<distance){best=i;distance=d;}}
 return best;
}
static void pixel(int x,int y,unsigned color){
 if((unsigned)x>=256||y<clip_y0||y>=clip_y1)return;
 unsigned offset=((y/8)*32+x/8)*32+(y&7)*4+(x&7)/2,shift=(x&1)*4;
 tiles[offset]=(tiles[offset]&~(15u<<shift))|(color<<shift);
}
static void rect(int x,int y,int w,int h,unsigned c){
 /* Radar terrain consists of aligned 8x8 cells; fill a complete native tile
  * directly instead of touching each packed pixel separately. */
 if(!((x|y|w|h)&7)&&x>=0&&x+w<=256&&y>=clip_y0&&y+h<=clip_y1){
  for(int j=y/8;j<(y+h)/8;++j)for(int i=x/8;i<(x+w)/8;++i)memset(tiles+(j*32+i)*32,c|(c<<4),32);
  return;
 }
 for(int j=0;j<h;++j)for(int i=0;i<w;++i)pixel(x+i,y+j,c);
}
static void border(int x,int y,int w,int h,unsigned c){rect(x,y,w,1,c);rect(x,y+h-1,w,1,c);rect(x,y,1,h,c);rect(x+w-1,y,1,h,c);}
static void text(int x,int y,const char* s,unsigned c,const unsigned char* font){
 if(!font)return;
 for(unsigned n=0;s[n];++n)for(unsigned j=0;j<8;++j)for(unsigned i=0;i<8;++i)
  if(font[(unsigned char)s[n]*8+j]&(1u<<i))pixel(x+n*8+i,y+j,c);
}
static void icon(unsigned character,unsigned id,int x,int y,int enabled){
 if(id>=18)return;
 for(unsigned j=0;j<24;++j)for(unsigned i=0;i<24;++i){unsigned c=x4_lower_icon_pixels[!!character][id][j*24+i];if(c)pixel(x+i,y+j,enabled?c:dim);}
}
void x4_lower_reset(void){mode=X4_LOWER_PANEL;selected=0;}
unsigned x4_lower_mode(void){return mode;}
unsigned x4_lower_selection(void){return selected;}
const unsigned short* x4_lower_palette(void){return x4_lower_icon_palette;}
int x4_lower_touch(unsigned x,unsigned y,const X4PlayerPanel* s){
 if(x>=256||y>=192)return -1;
 if(y>=168){mode=x<85?X4_LOWER_PANEL:x<170?X4_LOWER_DEBUG:X4_LOWER_MAP;return -1;}
 if(mode==X4_LOWER_DEBUG&&y>=144&&y<160)return x<85?X4_LOWER_OPEN_TEST:x<170?X4_LOWER_SIGMA_HITKILL:X4_LOWER_CREDITS;
 if(mode!=X4_LOWER_PANEL||!s->active)return -1;
 if(y>=24&&y<120){unsigned col=x/85,row=(y-24)/32;if(col>2)return -1;selected=row*3+col;
  if(!s->character&&(selected==0||(s->weapons&(1u<<(selected-1)))))return (int)selected;
 }else if(y>=120&&y<144){unsigned i=x/28;if(i<9)selected=9+i;}
 return -1;
}
static void tabs(const unsigned char* font){
 static const char* const labels[]={"PAINEL","DEBUG","MAPA"};
 for(unsigned i=0;i<3;++i){int x=i*85;rect(x,168,i==2?86:85,24,mode==i?dim:0);border(x,168,i==2?86:85,24,mode==i?cyan:dim);text(x+12,176,labels[i],mode==i?cyan:ink,font);}
}
const unsigned char* x4_lower_render(const X4PlayerPanel* s,const X4Radar* radar,unsigned muted,const unsigned char* font){
 static const char* const names[]={"X-Buster","Lightning Web","Frost Tower","Soul Body","Rising Fire","Ground Hunter","Aiming Laser","Double Cyclone","Twin Slasher"};
 static const char* const short_x[]={"BUSTER","L.WEB","FROST","SOUL","FIRE","HUNTER","LASER","CYCLON","SLASH"};
 static const char* const short_z[]={"SABER","RAIJIN","HYOURE","KUUEN","RYUEN","HIEN","RAKUHO","TENKUU","SHIPPU"};
 static const char* const skills[]={"Z-Saber","Raijingeki","Hyouretsuzan","Kuuenbu","Ryuenjin","Hienkyaku","Rakuhouha","Tenkuuha","Shippuuga"};
 static const char* const upgrades[]={"Cabeca","Corpo","Bracos","Pernas","Coracoes","Tanque vida 1","Tanque vida 2","Tanque armas","EX Tank"};
 memset(tiles,0,sizeof tiles);clip_y0=0;clip_y1=192;ink=nearest(31,31,31);dim=nearest(10,10,12);red=nearest(31,4,4);green=nearest(4,31,6);cyan=nearest(0,27,31);gold=nearest(31,26,0);
 char line[40];
 if(mode==X4_LOWER_MAP){
  rect(0,0,256,24,dim);text(4,4,"RADAR",cyan,font);text(72,4,"+Voce !Inimigo oItem",ink,font);text(4,14,"512 x 288  |  AREA ATUAL",ink,font);
  if(!radar->active)text(8,72,"Mapa disponivel na fase",ink,font);
  else{
   clip_y0=24;clip_y1=168;
   for(unsigned y=0;y<18;++y)for(unsigned x=0;x<32;++x)if(radar->cells[y][x])rect(x*8,24+y*8,8,8,dim);
   unsigned count=radar->marker_count;if(count>X4_RADAR_MAX_MARKERS)count=X4_RADAR_MAX_MARKERS;
   for(unsigned i=0;i<count;++i){const X4RadarMarker* m=&radar->markers[i];int x=m->x/2,y=24+m->y/2;
    if(m->x<0||m->x>=512||m->y<0||m->y>=288)continue;
    if(m->kind==1){rect(x-1,y-2,3,4,red);pixel(x,y+3,red);}else if(m->kind==2)border(x-2,y-2,5,5,gold);
   }
   int px=radar->player_x/2,py=24+radar->player_y/2;rect(px-3,py,7,1,cyan);rect(px,py-3,1,7,cyan);
  }
 }else{
  rect(0,0,256,10,dim);
  snprintf(line,sizeof line,"%s HP:%d/%d VIDAS:%d %s",s->character?"ZERO":"X",s->hp,s->max_hp,s->lives,muted?"MUDO":"");text(4,1,line,ink,font);
  unsigned hp=s->hp>0?(unsigned)s->hp:0,max=s->max_hp>0?(unsigned)s->max_hp:0;if(hp>max)hp=max;border(4,12,248,8,dim);if(max)rect(5,13,hp*246/max,6,green);
  if(!s->active)text(8,64,"Painel disponivel na fase",ink,font);
  else{
   unsigned card=nearest(6,8,12);
   for(unsigned i=0;i<9;++i){int x=(i%3)*85,y=24+(i/3)*32;int enabled=i==0||!!(s->weapons&(1u<<(i-1)));
    if(enabled)rect(x+1,y+1,82,29,card);
    border(x,y,84,31,selected==i?gold:!s->character&&s->weapon==(int)i?cyan:dim);icon(s->character,i,x+3,y+3,enabled);
    text(x+29,y+3,(s->character?short_z:short_x)[i],enabled?ink:dim,font);
    unsigned energy=s->energy[s->character?0:i]>48?48:s->energy[s->character?0:i];
    if(!enabled)snprintf(line,sizeof line,"--");
    else if((!s->character&&i)||(s->character&&i==6))snprintf(line,sizeof line,"%2u/48",energy);
    else snprintf(line,sizeof line,"LIVRE");
    text(x+30,y+12,line,enabled?ink:dim,font);
    if(!s->character&&i){unsigned e=s->energy[i]>48?48:s->energy[i];rect(x+30,y+24,49,3,dim);rect(x+30,y+24,e,3,enabled?green:dim);}
    else if(s->character&&i==6){unsigned e=s->energy[0]>48?48:s->energy[0];rect(x+30,y+24,49,3,dim);rect(x+30,y+24,e,3,enabled?green:dim);}
   }
   for(unsigned i=0;i<9;++i){int acquired=i<4?(!s->character&&!!(s->armor&(1u<<i))):i==4?!!(s->upgrades&255):!!(s->upgrades&(0x1000u<<(i-5)));
    icon(s->character,9+i,i*28+2,120,acquired);if(selected==9+i)border(i*28,120,27,24,gold);
   }
   rect(0,144,256,1,dim);
   if(selected<9){text(4,146,(s->character?skills:names)[selected],ink,font);
    unsigned owned=selected==0||!!(s->weapons&(1u<<(selected-1)));text(4,156,owned?(s->character?"Tecnica adquirida":"Toque equipa a arma"):"Poder nao adquirido",owned?cyan:dim,font);
   }else{unsigned i=selected-9;text(4,146,upgrades[i],ink,font);
    if(i==4){unsigned hearts=0;for(unsigned m=s->upgrades&255;m;m>>=1)hearts+=m&1;snprintf(line,sizeof line,"Coracoes: %u/8",hearts);}
    else if(i>=5&&i<=7){if(s->upgrades&(0x1000u<<(i-5)))snprintf(line,sizeof line,"Adquirido: %u/32",s->tanks[i-5]>32?32:s->tanks[i-5]);else snprintf(line,sizeof line,"Nao adquirido");}
    else if(i<4)snprintf(line,sizeof line,"%s",s->character?"Armadura exclusiva de X":s->armor&(1u<<i)?"Adquirido":"Nao adquirido");
    else snprintf(line,sizeof line,"%s",s->upgrades&0x8000?"Adquirido":"Nao adquirido");
    text(4,156,line,cyan,font);
   }
  }
 }
 clip_y0=0;clip_y1=192;tabs(font);return tiles;
}
