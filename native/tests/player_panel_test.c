#include "x4_player_panel.h"
#include <assert.h>
#include <string.h>
#include <stdio.h>
int main(void){
 char rows[X4_PANEL_ROWS][X4_PANEL_COLUMNS+1];
 X4PlayerPanel s={.active=1,.max_hp=48,.hp=24,.weapon=4,.weapons=8,.armor=15,.upgrades=0xf0ff,.lives=4};
 s.energy[4]=17;s.tanks[0]=32;
 x4_player_panel_rows(&s,rows,0);
 assert(strstr(rows[3],"24/48"));assert(strstr(rows[4],"##########----------"));
 assert(strstr(rows[10],"> Rising Fire"));assert(strstr(rows[10],"17/48"));
 assert(strstr(rows[7],"nao adquirida"));assert(strstr(rows[15],"8/8"));assert(strstr(rows[16],"4/4"));
 assert(strstr(rows[17],"32/32"));assert(strstr(rows[18]," 0/32"));
 assert(strstr(rows[19]," 0/32"));assert(strstr(rows[20],"EX Tank: adquirido"));
 X4PlayerPanel saved=s;x4_player_panel_rows(&s,rows,0);assert(!memcmp(&s,&saved,sizeof s));
 s.upgrades=0;s.tanks[0]=32;x4_player_panel_rows(&s,rows,0);
 assert(strstr(rows[17],"nao adquirido"));assert(strstr(rows[18],"nao adquirido"));
 assert(strstr(rows[19],"nao adquirido"));assert(strstr(rows[20],"nao adquirido"));
 s.upgrades=0xf0ff;
 s.character=1;s.weapons=255;s.hp=100;s.energy[0]=255;
 x4_player_panel_rows(&s,rows,1);
 assert(strstr(rows[3],"48/48"));assert(strstr(rows[12],"Rakuhouha"));assert(strstr(rows[12],"48/48"));
 assert(strstr(rows[7],"Raijingeki"));assert(strstr(rows[16],"8/8"));assert(strstr(rows[1],"OFF"));
 for(unsigned i=0;i<X4_PANEL_ROWS;++i){assert(strlen(rows[i])==X4_PANEL_COLUMNS);assert(!strchr(rows[i],'\n'));}
 s.active=0;x4_player_panel_rows(&s,rows,0);assert(strstr(rows[4],"disponivel"));
 s.active=1;
 s.hp=-1;s.max_hp=0;s.weapon=999;s.weapons=0;s.upgrades=0;
 x4_player_panel_rows(&s,rows,0);assert(strstr(rows[3]," 0/ 0"));assert(strstr(rows[4],"--------------------"));
 s.active=0;x4_player_panel_rows(&s,rows,0);assert(strstr(rows[4],"disponivel na fase"));
 for(unsigned i=0;i<X4_PANEL_ROWS;++i){assert(strlen(rows[i])==X4_PANEL_COLUMNS);assert(!strchr(rows[i],'\n'));}
 puts("PASS player panel: X/Zero, unlocks, energy, upgrades, inactive and bounds");

}
