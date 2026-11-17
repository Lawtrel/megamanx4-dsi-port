#include "x4_player_panel.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
static unsigned limit(unsigned n,unsigned max){return n<max?n:max;}
static void row(char* out,const char* format,...){
 char buffer[128];va_list args;va_start(args,format);vsnprintf(buffer,sizeof buffer,format,args);va_end(args);
 memset(out,' ',X4_PANEL_COLUMNS);size_t n=strlen(buffer);if(n>X4_PANEL_COLUMNS)n=X4_PANEL_COLUMNS;
 memcpy(out,buffer,n);out[X4_PANEL_COLUMNS]=0;
}
static void gauge(char* out,unsigned value,unsigned max){
 unsigned filled=max?limit(value,max)*20/max:0;
 for(unsigned i=0;i<20;++i)out[i]=i<filled?'#':'-';
 out[20]=0;
}
static unsigned bits(unsigned n){unsigned result=0;while(n){result+=n&1;n>>=1;}return result;}
void x4_player_panel_rows(const X4PlayerPanel* s,char lines[X4_PANEL_ROWS][X4_PANEL_COLUMNS+1],unsigned muted){
 static const char* const weapons[]={"Lightning Web","Frost Tower","Soul Body","Rising Fire","Ground Hunter","Aiming Laser","Double Cyclone","Twin Slasher"};
 static const char* const skills[]={"Raijingeki","Hyouretsuzan","Kuuenbu","Ryuenjin","Hienkyaku","Rakuhouha","Tenkuuha","Shippuuga"};
 for(unsigned i=0;i<X4_PANEL_ROWS;++i)row(lines[i],"");
 row(lines[0],"PAINEL  | toque: diagnostico");
 row(lines[1],"MEGA MAN X4     Som: %s",muted?"OFF":"ON");
 if(!s->active){row(lines[4],"Painel disponivel na fase.");row(lines[6],"Use o menu na tela superior.");return;}
 unsigned max=limit(s->max_hp<0?0:(unsigned)s->max_hp,48),hp=limit(s->hp<0?0:(unsigned)s->hp,max);
 row(lines[2],"%s  Fase %02d/%d  Vidas: %d",s->character?"Zero":"X",s->stage,s->area+1,s->lives);
 row(lines[3],"VIDA %2u/%2u",hp,max);char bar[21];gauge(bar,hp,max);row(lines[4],"[%s]",bar);
 row(lines[5],s->character?"TECNICAS  (energia: Rakuhouha)":"ARMAS  > equipada   energia/48");
 row(lines[6],s->character?" Z-Saber             ilimitado":"%c X-Buster           ilimitado",s->weapon==0?'>':' ');
 for(unsigned i=0;i<8;++i){
  if(!(s->weapons&(1u<<i)))row(lines[7+i]," %u: nao adquirida",i+1);
  else if(s->character&&i!=5)row(lines[7+i]," %-17s OK",skills[i]);
  else row(lines[7+i],"%c %-17s %2u/48",!s->character&&s->weapon==(int)i+1?'>':' ',s->character?skills[i]:weapons[i],limit(s->energy[s->character?0:i+1],48));
 }
 row(lines[15],"UPGRADES  Coracoes: %u/8",bits(s->upgrades&255));
 if(s->character)row(lines[16],"Tecnicas adquiridas: %u/8",bits(s->weapons&255));
 else row(lines[16],"Armadura: %u/4  Giga: %2u/48",bits(s->armor&15),limit(s->energy[0],48));
 for(unsigned i=0;i<3;++i){unsigned mask=0x1000u<<i;
  if(s->upgrades&mask)row(lines[17+i],"%s: %2u/32",i==0?"Tanque vida 1":i==1?"Tanque vida 2":"Tanque armas ",limit(s->tanks[i],32));
  else row(lines[17+i],"%s: nao adquirido",i==0?"Tanque vida 1":i==1?"Tanque vida 2":"Tanque armas ");
 }
 row(lines[20],"EX Tank: %s",s->upgrades&0x8000?"adquirido":"nao adquirido");
 row(lines[22],"L/R:armas  START:menu");
 row(lines[23],"SELECT+X:som  Topo:diagnostico");
}
