/* Original game state machine driven by native cooperative tasks.
 * A separate workbench from the physically tested N05 DEV slice. */
#include "common.h"
#include "main/2824.h"
#include "main/323C.h"
#include "main/102FC.h"
#include "main/objects.h"
#include "main/stage_layouts.h"
#include "main/55C4.h"
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
static unsigned debug_loadout;
static char root_path[1024];
extern int x4_original_camera_mode;
extern void x4_flow_ui_begin_frame(void);
extern void x4_flow_ui_reset(void);
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
 debug_loadout=0;
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
int x4_flow_step(unsigned held,unsigned pressed){
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
 ++main_bss_state.frame_counter;++frames;x4_flow_ui_begin_frame();clear_vram_rect_ptrs();func_800169D8();func_80012600();
 if(debug_loadout&&g_Player.active)g_Player.boss_flags=(s8)0xFF;
 return x4_flow_load_error()?x4_flow_load_error():x4_n05_platform_error();
}
void x4_flow_state(X4FlowState* out){
 *out=(X4FlowState){engine_obj.state,engine_obj.unk1,engine_obj.stage,engine_obj.substage,engine_obj.cur_character,
 engine_obj.checkpoint,g_Player.x_pos.i.hi,g_Player.y_pos.i.hi,(u8)g_Player.hp&127,g_Player.state,engine_obj.gameplay_locked,
 frames,draw_errors,(unsigned)!!x4_flow_load_error()};
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
