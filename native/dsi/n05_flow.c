/* Original game state machine driven by native cooperative tasks.
 * A separate workbench from the physically tested N05 DEV slice. */
#include "common.h"
#include "main/2824.h"
#include "main/323C.h"
#include "main/102FC.h"
#include "main/objects.h"
#include "main/stage_layouts.h"
#include "main/55C4.h"
#include "main/player_weapon.h"
#include "main/object_update.h"
#include "x4_flow.h"
#include "x4_n05_actors.h"
#include "x4_n05_platform.h"
#include "x4_stage_spawn.h"
#include "x4_sfx.h"
#include "x4_video.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
extern void x4_flow_loading_init(const char*);
extern void x4_flow_loading_close(void);
extern void func_80012600(void),clear_vram_rect_ptrs(void),func_8001FB50(void);
extern u8* D_8010FFDC[17][2];
extern u16 D_800EE430[16];
static unsigned frames,draw_errors,draw_requested;
static unsigned fade_alpha;
static unsigned debug_loadout,debug_sigma_hitkill;
static int requested_weapon=-1;
static char root_path[1024];
extern int x4_original_camera_mode;
extern void x4_flow_ui_begin_frame(void);
extern void x4_flow_ui_reset(void);
extern void func_800170E0(void);
extern int x4_flow_ui_draw(uint16_t*,const X4Archive*);
extern int x4_n05_draw_ui_overlay(uint16_t*,const X4Archive*);
extern int x4_n06_draw_hud(uint16_t*,const X4Archive*);
void x4_flow_copy_layout(void){
 assert((unsigned)engine_obj.stage<17&&(unsigned)engine_obj.substage<2);
 assert(layout_size>=0&&(unsigned)layout_size*3<=PC_TILEMAP_SIZE);
 assert(D_8010FFDC[(u8)engine_obj.stage][(u8)engine_obj.substage]);
 /* Original stage scripts address D_80141BE8 directly (train cars, ice
  * effects). Drawing and collision must share that same writable map. */
 memcpy(D_80141BE8,D_8010FFDC[(u8)engine_obj.stage][(u8)engine_obj.substage],layout_size*3);
 SP_BG_TILEMAP=D_80141BE8;
}
/* Native rendering reads the live layout, rather than creating Sony GPU
 * primitives. These engine entry points request a native frame submission. */
void func_80023CE0(void){x4_flow_copy_layout();draw_requested=1;}
void func_80023D68(void){x4_n05_background_wrap();draw_requested=1;}
void init_objects(void){x4_n05_background_wrap();draw_requested=1;}
void func_80024260(void){draw_requested=1;}
void func_80024E70(void){extern void func_80017100(void),func_80017340(void);func_80017100();func_80017340();draw_requested=1;}
void func_80026118(void){x4_flow_copy_layout();}
/* Original fade task, with its output applied to the native framebuffer. */
void func_80012A3C(void){
 int phase=0,settle=0;
 fade_alpha=main_bss_state.transition.active>0?255:0;
 for(;;){
  func_800127C8(1);
  if(!phase){
   int a=(int)fade_alpha+(main_bss_state.transition.active>0?-1:1)*(s8)main_bss_state.transition.fade_amount;
   if(a<=0||a>=255){a=a<=0?0:255;main_bss_state.transition.fade_amount=2;phase=1;}
   fade_alpha=a;++settle;
  }else if(!main_bss_state.transition.fade_amount){main_bss_state.transition.active=0;func_800127FC();}
  else --main_bss_state.transition.fade_amount;
  draw_requested=1;
 }
}
void func_80013530(void){
 func_800129F0(16);
 while(main_bss_state.transition.active)func_800127C8(1);
}
uint8_t mmx4_pc_read_bg_tilemap(const uint8_t* map,int32_t offset){return map&&offset>=0&&offset<layout_size*3?map[offset]:0;}
int mmx4_pc_hide_background;
void mmx4_pc_render_log_dump(void){fprintf(stderr,"Native draw diagnostic: frame=%u errors=%u\n",frames,draw_errors);}
/* Use the bounds-checked native record activation in the full engine too. */
void func_80028FEC(s16 l,s16 r,s16 t,s16 b,u8 direction){
 assert(x4_stage_spawn_visible(l,r,t,b,direction)>=0);
}
void x4_flow_close(void){
 debug_loadout=0;debug_sigma_hitkill=0;requested_weapon=-1;
 x4_flow_ui_reset();
 x4_n05_set_common_graphics(NULL);x4_flow_loading_close();x4_n05_platform_close();
}
int x4_flow_begin(const char* root,unsigned character,unsigned stage,unsigned area){
 if(!root||strlen(root)>=sizeof root_path||character>1||stage>12||area>1||((stage==9||stage==10)&&area))return -1;
 x4_flow_close();strcpy(root_path,root);x4_n05_platform_init(root_path);x4_flow_loading_init(root_path);x4_sfx_character(character);
 mmx4_pc_threads_init();reset_game_engine();reset_objects();
 /* The native entry bypasses func_8001213C's device setup, including its
  * RNG seed. Zero is an absorbing state and hangs ice_core_shed_ice when
  * piece zero is absent. Use the original startup seed for every new run. */
 cur_random=0xD37;
 memset(&main_bss_state,0,sizeof main_bss_state);memset(&controller_input,0,sizeof controller_input);
 frames=draw_errors=0;draw_requested=0;fade_alpha=255;x4_original_camera_mode=1;
 if(x4_flow_load(0x40,0,NULL)||x4_flow_load(0x41,2,WINDOW_ARCHIVE_DATA))return -2;
 if(x4_flow_load(character?0x4d:0x4b,0,NULL))return -3;
 D_80173C80=D_8015D9C8;saved_vab_address=loaded_vab_address;
 engine_obj.cur_character=character;engine_obj.stage=stage;engine_obj.substage=area;
 engine_obj.state=4;engine_obj.player_lives=2;engine_obj.max_hp=32;
 /* A connected native controller must not trigger the original unplug menu. */
 pad_port1_packet[0]=0;pad_port1_packet[1]=0x41;
 func_80012740(0,func_8001FB50);return 0;
}
int x4_flow_begin_debug(const char* root,unsigned character,unsigned stage,unsigned area){
 int result=x4_flow_begin(root,character,stage,area);if(result)return result;
 debug_loadout=1;
 /* Original item limits: eight hearts, two full life tanks, weapon tank and
  * EX Tank. Grant equipment once; damage and energy use remain original. */
 engine_obj.max_hp=engine_obj.checkpoint_hp=0x30;
 engine_obj.player_lives=4;engine_obj.collected_upgrades=0xF0FF;
 engine_obj.sub_tanks[0]=(s8)0xA0;engine_obj.sub_tanks[1]=(s8)0xA0;engine_obj.sub_tanks[2]=0x20;
 memset(engine_obj.player_initial_data,0x30,sizeof engine_obj.player_initial_data);
 if(character==0){engine_obj.armor_parts=0x0F;engine_obj.arm_type=2;}
 return 0;
}
int x4_flow_new_game(const char* root){int result=x4_flow_begin(root,0,0,0);if(!result)engine_obj.state=0;return result;}
int x4_flow_title(const char* root){
 int result=x4_flow_begin(root,0,0,0);if(result)return result;
 extern void func_8001DC30(void);
 memset(&game_info,0,sizeof game_info);func_8001DC30();return 0;
}

static int weapon_selection_allowed(void){
 return engine_obj.state==6&&engine_obj.cur_character==0&&g_Player.active&&((u8)g_Player.hp&127)&&
  !g_Player.input_locked&&!g_Player.is_clone&&!g_Player.special_shot_count&&!g_Player.ride_state&&
  !g_Player.nova_strike_active&&!engine_obj.gameplay_locked;
}
int x4_flow_request_weapon(unsigned weapon){
 if(weapon>8||engine_obj.cur_character!=0||(weapon&&!((u8)g_Player.boss_flags&(1u<<(weapon-1)))))return -1;
 if(!weapon_selection_allowed())return -2;
 requested_weapon=(int)weapon;return 0;
}
static void consume_weapon_request(void){
 int target=requested_weapon;requested_weapon=-1;
 if(target<0||!weapon_selection_allowed()||(target&&!((u8)g_Player.boss_flags&(1u<<(target-1)))))return;
 /* Reuse the original cycling function, preserving its charge reset, stock
  * charge, sound and restrictions. No synthetic input reaches the scheduler. */
 int original_weapon=g_Player.weapon;
 u16 saved_pressed=g_Player.pressed_input,saved_held=g_Player.input.buttons.held;
 g_Player.pressed_input=0x200;g_Player.input.buttons.held=0;
 for(unsigned n=0;n<9&&g_Player.weapon!=target;++n){
  int before=g_Player.weapon;func_80036F50(&g_Player);if(g_Player.weapon==before)break;
 }
 g_Player.pressed_input=saved_pressed;g_Player.input.buttons.held=saved_held;
 /* Complete the same post-selection work as player_update_weapon. Doing
  * this once for the final target avoids spawning intermediate weapons. */
 player_update_shot_types(&g_Player);
 if(original_weapon!=g_Player.weapon)player_equip_weapon(&g_Player);
}
static void debug_sigma_prepare(void);
void x4_flow_debug_sigma_hitkill(int enabled){debug_sigma_hitkill=!!enabled;debug_sigma_prepare();}
int x4_flow_debug_sigma_hitkill_enabled(void){return debug_sigma_hitkill;}
static void debug_sigma_prepare(void){
 if(!debug_sigma_hitkill||engine_obj.state!=6||engine_obj.stage!=12||engine_obj.substage!=1||
  !engine_obj.enable_boss||engine_obj.gameplay_locked)return;
 struct MainObj* boss=engine_obj.boss_ptr;
 uintptr_t p=(uintptr_t)boss,start=(uintptr_t)main_objects;
 if(p<start||p-start>=sizeof main_objects||(p-start)%sizeof *boss)return;
 /* Both real Sigma actors use state1 for battle. Intro HP filling and
  * scripted reveal/death states must never be rewritten by this cheat. */
 if(!boss->active||boss->state!=1||boss->hp<=1)return;
 if(boss->id==0x44||(boss->id==0x4a&&boss->ext.main_74.target))boss->hp=1;
}
int x4_flow_debug_credits(void){
 if(!root_path[0]||(unsigned)engine_obj.cur_character>1)return -1;
 char content[sizeof root_path];strcpy(content,root_path);
 unsigned character=(u8)engine_obj.cur_character;
 /* Recreate the native task arena instead of overwriting a suspended boss
  * script/menu coroutine. The stored save journal is never touched. */
 int error=x4_flow_begin(content,character,1,0);
 if(!error){engine_obj.state=11;engine_obj.unk1=0;engine_obj.unk2=0;}
 return error;
}
int x4_flow_step(unsigned held,unsigned pressed){
 consume_weapon_request();debug_sigma_prepare();
 controller_input.previous=controller_input.held;
 controller_input.held=controller_input.pressed=0;
 for(unsigned i=0;i<16;++i){if(held&(1u<<i))controller_input.held|=D_800EE430[i];if(pressed&(1u<<i))controller_input.pressed|=D_800EE430[i];}
 /* Dedicated native frontend buttons, outside the original action table. */
 if(held&(1u<<16))controller_input.held|=PADstart;if(pressed&(1u<<16))controller_input.pressed|=PADstart;
 if(held&(1u<<17))controller_input.held|=PADselect;if(pressed&(1u<<17))controller_input.pressed|=PADselect;
 /* Weapons/Zero techniques share bits with defeated bosses in the original
  * save. Override only the live player so debug still exercises boss fights.
  * Apply on both sides of the scheduler because player_spawn restores bits. */
 if(debug_loadout&&g_Player.active)g_Player.boss_flags=(s8)0xFF;
 unsigned previous_state=(u8)engine_obj.state;
 ++main_bss_state.frame_counter;++frames;x4_flow_ui_begin_frame();clear_vram_rect_ptrs();func_800169D8();func_80012600();
 /* Game-over windows borrow the live RAM palette in the native renderer.
  * Original Continue only requests a later VRAM palette upload, so no
  * original restore callback runs here. Restore before the next state's
  * loading/spawn work. State 12 is a nested save menu, still using the window. */
 if(previous_state==8&&engine_obj.state!=8&&engine_obj.state!=12)func_800170E0();
 if(debug_loadout&&g_Player.active)g_Player.boss_flags=(s8)0xFF;
 return x4_flow_load_error()?x4_flow_load_error():x4_n05_platform_error();
}
void x4_flow_state(X4FlowState* out){
 *out=(X4FlowState){engine_obj.state,engine_obj.unk1,engine_obj.stage,engine_obj.substage,engine_obj.cur_character,
 engine_obj.checkpoint,g_Player.x_pos.i.hi,g_Player.y_pos.i.hi,(u8)g_Player.hp&127,g_Player.state,engine_obj.gameplay_locked,
 frames,draw_errors,(unsigned)!!x4_flow_load_error()};
}
void x4_flow_player_panel(X4PlayerPanel* out){
 memset(out,0,sizeof *out);
 out->active=engine_obj.state==6&&g_Player.active;
 out->character=engine_obj.cur_character;out->stage=engine_obj.stage;out->area=engine_obj.substage;
 out->hp=(u8)g_Player.hp&127;out->max_hp=(u8)engine_obj.max_hp;
 out->lives=engine_obj.player_lives;out->weapon=(u8)g_Player.weapon;
 /* Same live unlock bits and indices used by player_weapon_bits/pause_menu.
  * Debug grants affect only g_Player, so use that source for the panel too. */
 out->weapons=(u8)g_Player.boss_flags;out->armor=(u8)engine_obj.armor_parts;
 /* Pickup bits0..7: hearts; bits12..14: tanks; bit15: EX.
  * Tank bit7 is an original menu flag rather than stored energy. */
 out->upgrades=engine_obj.collected_upgrades;
 for(unsigned i=0;i<9;++i)out->energy[i]=(u8)g_Player.weapon_energy[i];
 for(unsigned i=0;i<3;++i)out->tanks[i]=(u8)engine_obj.sub_tanks[i]&127;
}

static int radar_origin(int player,int visible,int extent){
 int value=(player-visible/2)&~15;
 int maximum=extent>visible?extent-visible:0;
 if(value<0)value=0;if(value>maximum)value=maximum;
 return value;
}
static void radar_marker(X4Radar* out,int x,int y,unsigned kind){
 x-=out->origin_x;y-=out->origin_y;
 if(x<0||x>=512||y<0||y>=288||out->marker_count>=X4_RADAR_MAX_MARKERS)return;
 out->markers[out->marker_count++]=(X4RadarMarker){x,y,kind};
}
void x4_flow_radar(X4Radar* out){
 memset(out,0,sizeof *out);
 if(engine_obj.state!=6||!g_Player.active||!layout_width||!layout_height||!SP_BG_TILEMAP)return;
 int layer=g_Player.bg_offset;if(layer<0||layer>=3)return;
 const X4Archive* archive=x4_flow_archive();
 const X4ArchiveMember* tiles=x4_archive_find(archive,0,0);
 const X4ArchiveMember* attrs=x4_archive_find(archive,1,0);
 if(!tiles||!attrs||!SP_BG_TILE_PIXELS||!SP_BG_TILE_ATTRS||layout_size<(unsigned)layout_width*layout_height)return;
 out->active=1;
 out->origin_x=radar_origin(g_Player.x_pos.i.hi,512,(int)layout_width*256);
 out->origin_y=radar_origin(g_Player.y_pos.i.hi,288,(int)layout_height*256);
 out->player_x=g_Player.x_pos.i.hi-out->origin_x;out->player_y=g_Player.y_pos.i.hi-out->origin_y;
 for(unsigned row=0;row<X4_RADAR_ROWS;++row)for(unsigned col=0;col<X4_RADAR_COLUMNS;++col){
  unsigned x=out->origin_x+col*16,y=out->origin_y+row*16;
  if(x>=(unsigned)layout_width*256||y>=(unsigned)layout_height*256)continue;
  unsigned offset=layer*layout_size+(y/256)*layout_width+x/256;
  if(offset>=(unsigned)layout_size*3)continue;
  unsigned block=SP_BG_TILEMAP[offset];if(!block)continue;
  unsigned index=block*256+((y/16)&15)*16+((x/16)&15);
  if(index>=tiles->size/2)continue;
  unsigned tile=SP_BG_TILE_PIXELS[index]&0x3fff;if(tile>=attrs->size/4)continue;
  out->cells[row][col]=(uint8_t)SP_BG_TILE_ATTRS[tile]!=0;
 }
 for(unsigned i=0;i<COUNT(main_objects);++i){
  const struct MainObj* object=&main_objects[i];
  if(object->active&&object->on_screen&&object->hp>0&&object->hurt_box&&object->bg_offset==layer)
   radar_marker(out,object->x_pos.i.hi,object->y_pos.i.hi,1);
 }
 for(unsigned i=0;i<COUNT(item_objects);++i){
  const struct ItemObj* object=&item_objects[i];
  /* Only actual visible pickups; the item array also owns doors, platforms
   * and hidden/destructible structures that are not radar collectibles. */
  if(object->active&&object->on_screen&&object->id==2&&object->state==1&&object->bg_offset==layer)
   radar_marker(out,object->x_pos.i.hi,object->y_pos.i.hi,2);
 }
}
void x4_flow_debug(char* text,unsigned capacity){
 extern u16 g_FilterAmountG;
 struct MainObj* boss=engine_obj.enable_boss?engine_obj.boss_ptr:NULL;
 int death=-1,timer=0;
 for(unsigned i=0;i<COUNT(effect_objects);++i)if(effect_objects[i].active&&effect_objects[i].id==26){death=effect_objects[i].state;timer=effect_objects[i].ext.effect_26.timer;break;}
 unsigned bg[3],armor;x4_n05_render_counts(bg,&armor);
 snprintf(text,capacity,"BOSS enabled=%d hp=%d state=%d/%d/%d death=%d timer=%d filter=%u/%u/%u script=%d dialogue=%d menu=%d/%d/%d bg=%u/%u/%u armor=%u mask=%u",
  engine_obj.enable_boss,boss?boss->hp:-1,boss?boss->state:-1,boss?boss->unk5:-1,boss?boss->unk6:-1,
  death,timer,g_FilterAmountR,g_FilterAmountG,g_FilterAmountB,g_Player.script_state,abc_object.unkC,bar_object.state,bar_object.unk5,bar_object.unk6,bg[0],bg[1],bg[2],armor,(u8)g_Player.armor_parts);
}
int x4_flow_draw(uint16_t* screen){
 if(!draw_requested)return 0;
 draw_requested=0;const X4Archive* a=x4_flow_archive();
 int errors=x4_n05_draw_scene(screen,a,a,background_objects[0].x_pos.i.hi,background_objects[0].y_pos.i.hi);
 errors+=x4_flow_ui_draw(screen,a);
 errors+=x4_n05_draw_ui_overlay(screen,a);
 errors+=x4_n06_draw_hud(screen,a);
 draw_errors+=errors;
 if(x4_video_enabled)x4_video_fade(fade_alpha);
 else if(fade_alpha)for(unsigned i=0;i<256*192;++i){unsigned c=screen[i],gain=255-fade_alpha;
  screen[i]=0x8000|(((c&31)*gain/255))|((((c>>5)&31)*gain/255)<<5)|((((c>>10)&31)*gain/255)<<10);
 }
 return errors;
}
