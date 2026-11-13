/* Native UI rendering from the game's own tile maps and sprite assemblies.
 * Replaces high-level Sony drawing routines; emits no GPU command packets. */
#include "common.h"
#include "x4_flow.h"
#include "x4_video.h"
#include <assert.h>
#include <string.h>
struct UiRect {int x,y,w,h,u,v;unsigned image,clut;};
struct UiSprite {struct AnimatedObj object;const u8* frames;};
static struct UiRect rectangles[32];static unsigned rect_count;
static struct UiSprite sprites[64];static unsigned sprite_count;
static int tile_background;
static struct {int x0,y0,x1,y1,x2,y2,x3,y3;} gauges[14];static unsigned gauge_count;
static u16 saved_menu_palette[512];static int menu_palette_active;
static u16 saved_pause_palette[512];static int pause_palette_active;
extern u16 D_800F1868[18];
extern union MainPaletteData D_800F1660;
extern int x4_n05_draw_ui_sprite(uint16_t*,const X4Archive*,const struct AnimatedObj*,const u8*);
struct NativeHudLayout {s16 x,y;u8 tile,character,clut,alternate_clut;};
struct MovieHudQuad {s16 x0,y0,x1,y1,x2,y2,x3,y3,index;};
extern struct NativeHudLayout D_800F1AD8[38];extern s16 D_800F1C08[2];
void x4_flow_ui_begin_frame(void){rect_count=sprite_count=gauge_count=0;tile_background=0;}
void x4_flow_ui_reset(void){menu_palette_active=pause_palette_active=0;x4_flow_ui_begin_frame();}
#ifdef __linux__
int x4_flow_ui_pause_palette_active(void){return pause_palette_active;}
#endif
void func_800160F4(void){assert(SP_PALETTE);memcpy(SP_PALETTE,&D_800F1660,512);need_palette_load|=1;}
void func_800170B0(void){
 assert(SP_PALETTE&&SP_VRAM_IMAGE);
 if(!pause_palette_active)memcpy(saved_pause_palette,SP_PALETTE+1024,sizeof saved_pause_palette);
 pause_palette_active=1;memcpy(SP_PALETTE+1024,SP_VRAM_IMAGE,sizeof saved_pause_palette);need_palette_load|=1;
}
void func_800170E0(void){
 /* The original merely re-uploaded the unchanged game palette from RAM.
  * Our native pause renderer temporarily changes that RAM palette, so it
  * must restore it explicitly rather than only setting an upload flag. */
 if(pause_palette_active){assert(SP_PALETTE);memcpy(SP_PALETTE+1024,saved_pause_palette,sizeof saved_pause_palette);pause_palette_active=0;}
 need_palette_load|=1;
}
void func_8001E980(u8 restore){
 assert(SP_PALETTE);
 if(!restore){
  unsigned offset=((u32*)WINDOW_ARCHIVE_DATA)[1];assert(offset<=sizeof window_archive_data-1024);
  if(!menu_palette_active)memcpy(saved_menu_palette,SP_PALETTE,sizeof saved_menu_palette);
  memcpy(SP_PALETTE,WINDOW_ARCHIVE_DATA+offset,sizeof saved_menu_palette);menu_palette_active=1;
 }else if(menu_palette_active){memcpy(SP_PALETTE,saved_menu_palette,sizeof saved_menu_palette);menu_palette_active=0;}
 need_palette_load|=1;
}
void func_80016124(void){
 int x=0,y=0;
 for(unsigned i=0;i<17;++i){int type=(s16)D_800F1868[i],w=type==0?112:type==1?96:16;
  assert(rect_count<32);rectangles[rect_count++]=(struct UiRect){x,y,w,48,type==1?112:0,208,0x10008,0x784c};
  x+=w;if(x>=320){x=0;y+=48;}
 }
}
void func_80017100(void){extern void func_80023D68(void);tile_background=1;func_80023D68();}
void func_800175AC(u8 index){
 if(index>38||sprite_count>=64)return;
 struct NativeHudLayout layout;
 if(index<38)layout=D_800F1AD8[index];
 else{layout.x=D_800F1C08[0];layout.y=D_800F1C08[1];const u8* p=(const u8*)&g_BootTransitionDataRegion;
  layout.tile=p[0];layout.character=p[1];layout.clut=p[2];layout.alternate_clut=p[3];}
 if(index==0x20){layout.y=216;layout.x=((s8)engine_obj.max_hp<<1)+128;}
 unsigned pose=layout.tile;
 if(index==0x1f)pose+=engine_obj.player_lives;
 if(index==0x26&&bar_object.options.fields.unk25)pose+=10;
 unsigned value=layout.clut;
 if(bar_object.unk14==layout.character)value=main_bss_state.frame_counter&16?layout.alternate_clut:layout.clut;
 else if(engine_obj.cur_character==0){if(g_Player.weapon+1==layout.character||(!g_Player.weapon&&!layout.character))value=layout.alternate_clut;}
 else value=index<0x21?layout.alternate_clut:layout.clut;
 if(!engine_obj.cur_character&&layout.character==1)value=g_Player.weapon_energy[0]<48?layout.clut:layout.alternate_clut;
 struct UiSprite* s=&sprites[sprite_count++];memset(s,0,sizeof *s);
 s->object.active=s->object.on_screen=1;s->object.bg_offset=-1;
 s->object.x_pos.i.hi=layout.x;s->object.y_pos.i.hi=layout.y;
 s->object.animation_step.fields.frame_index=pose;s->object.unk40=0x1d60;
 s->object.unk42=(value&15)|(((value>>4)+480)<<6);s->frames=SP_TITLE_FRAMES;
}
/* Native equivalent of the pause display's high-level assembly. The original
 * ordering-table submit path is not used because no GPU packets are created. */
void func_800179BC(void){
 extern struct MovieHudQuad D_800F1C18[];
 for(unsigned i=0;i<14;++i){
  if(i<13&&!bar_object.options.items[i])continue;
  if(i&&i<9&&engine_obj.cur_character)continue;
  if(gauge_count>=14)break;
  int x0,y0,x1,y1,x2,y2,x3,y3;
  if(i==13){x0=110;y0=210;x2=103;y2=217;x1=x0+(s8)g_Player.hp*2;y1=210;x3=x2+(s8)g_Player.hp*2;y3=217;}
  else{
   const struct MovieHudQuad* r=&D_800F1C18[i];x0=r->x0;y0=r->y0;x1=r->x1;y1=r->y1;x2=r->x2-1;y2=r->y2;x3=r->x3;y3=r->y3;
   int amount=-1;
   if(i<10){if(i>=2||(i==0)!=(engine_obj.cur_character==0))amount=g_Player.weapon_energy[i<2?0:i-1]*2-1;}
   else amount=engine_obj.sub_tanks[i-10]&(i==12?255:127);
   if(amount>=0){x1=x0+amount;x3=x2+amount;}else if(amount== -1&&i!=0){x1=x0;x3=x2;}
  }
  gauges[gauge_count++]=(typeof(gauges[0])){x0,y0,x1,y1,x2,y2,x3,y3};
 }
}
void func_80017340(void){
 unsigned end=engine_obj.cur_character?8:10;
 if(!engine_obj.cur_character){func_800175AC(20);func_800175AC(0);func_800175AC(1);
  if(bar_object.options.items[1]){func_800175AC(21);func_800175AC(2);func_800175AC(3);}}
 for(unsigned i=engine_obj.cur_character?0:2;i<end;++i)if(bar_object.options.items[i]){func_800175AC(i+20);func_800175AC(i*2);func_800175AC(i*2+1);}
 if(bar_object.options.fields.unk20)func_800175AC(35);if(bar_object.options.fields.unk21)func_800175AC(36);if(bar_object.options.fields.unk22)func_800175AC(37);
 func_800175AC(31);if(bar_object.options.fields.unk25)func_800175AC(38);func_800175AC(30);func_800175AC(32);
 if(engine_obj.stage>=1&&engine_obj.stage<=8){unsigned i=engine_obj.cur_character?D_800F1C0F[(u8)engine_obj.stage]:engine_obj.stage+1;if(bar_object.options.items[i])func_800175AC(33);}
 func_800175AC(34);func_800179BC();
}
static int image_rect(uint16_t* out,const X4Archive* a,const struct UiRect* r){
 const X4ArchiveMember* image=x4_archive_find(a,r->image,0);if(!image||!SP_PALETTE)return 1;
 unsigned pi=((r->clut>>6)-480)*256+(r->clut&63)*16;if(pi+16>2048)return 1;
 if(x4_video_enabled){
  x4_video_screen_space(1);x4_video_order(-8192);
  for(int y=0;y<r->h;y+=16)for(int x=0;x<r->w;x+=16){
   unsigned u=(r->u+x)&255,v=(r->v+y)&255,w=r->w-x<16?r->w-x:16,h=r->h-y<16?r->h-y:16;
   unsigned offset=v*128+u/2;
   if(offset+(h-1)*128+(w+1)/2>image->size)return 1;
   if(x4_video_tile(image->data+offset,128,4,SP_PALETTE+pi,16,r->x+x,r->y+y,w,h,0,0,0))return 1;
  }return 0;
 }
 for(int y=0;y<r->h;++y)for(int x=0;x<r->w;++x){
  int px=(r->x+x)*4/5,py=(r->y+y)*4/5;if(px<0||px>=256||py<0||py>=192)continue;
  unsigned u=(r->u+x)&255,v=(r->v+y)&255,offset=v*128+u/2;if(offset>=image->size)return 1;
  unsigned index=(image->data[offset]>>((u&1)*4))&15;u16 color=SP_PALETTE[pi+index];if(color)out[py*256+px]=color|0x8000;
 }
 return 0;
}
int x4_flow_ui_draw(uint16_t* out,const X4Archive* a){
 int errors=0;
 if(tile_background){
  const X4ArchiveMember* tilemap=x4_archive_find(a,16,0);if(!tilemap||tilemap->size<1024)return 1;
  const u16* tiles=(const u16*)tilemap->data;
  for(unsigned page=0;page<2;++page)for(unsigned y=0;y<16;++y)for(unsigned x=0;x<16;++x){
   unsigned tile=tiles[page*256+y*16+x];struct UiRect r={(int)(page*256+x*16),(int)y*16,16,16,(tile&15)*16,tile&240,0x10007,(tile>>12)|0x7900};
   errors+=image_rect(out,a,&r);
  }
 }
 for(unsigned i=0;i<rect_count;++i)errors+=image_rect(out,a,&rectangles[i]);
 for(unsigned i=0;i<sprite_count;++i)errors+=x4_n05_draw_ui_sprite(out,a,&sprites[i].object,sprites[i].frames);
 for(unsigned i=0;i<gauge_count;++i){
  if(x4_video_enabled){
   x4_video_screen_space(1);x4_video_order(-8194);
   typeof(gauges[0])* g=&gauges[i];int x[4]={g->x0,g->x1,g->x3,g->x2},y[4]={g->y0,g->y1,g->y3,g->y2};
   uint16_t c[4]={0x1705,0x1705,0x1705,0x1705};errors+=x4_video_quad(x,y,c,0);continue;
  }
  typeof(gauges[0])* g=&gauges[i];int top=g->y0*4/5,bottom=g->y2*4/5;
  if(bottom<=top)continue;
  for(int y=top;y<bottom;++y){if(y<0||y>=192)continue;
   int l=(g->x0+(g->x2-g->x0)*(y-top)/(bottom-top))*4/5;
   int r=(g->x1+(g->x3-g->x1)*(y-top)/(bottom-top))*4/5;
   if(l<0)l=0;if(r>256)r=256;for(int x=l;x<r;++x)out[y*256+x]=0x9705;
  }
 }
 return errors;
}
