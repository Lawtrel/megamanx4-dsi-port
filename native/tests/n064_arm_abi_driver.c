/* Isolated ARM instruction test: actual death caller and effect-pool cleanup.
 * No console, GPU, BIOS or game assets. Support calls have no side effects. */
#include "common.h"
#include "main/1AC84.h"
#include "main/objects.h"
struct EffectObj effect_objects[32];
struct EngineObj engine_obj;
u16 g_FilterAmountR,g_FilterAmountG,g_FilterAmountB;
u8 g_FilterModeR,g_FilterModeG,g_FilterModeB;
s32 D_8013E188[4];
struct QuadObj g_QuadObjects[32];
void spawn_explosion(struct BaseObj* object){(void)object;}
void func_80016F0C(void){}
extern void func_800BB364(struct EffectObj*);
unsigned unit_entry(void){
 for(unsigned i=0;i<3;++i){effect_objects[i].active=1;effect_objects[i].id=i<2?2:7;effect_objects[i].unk2=i+7;}
 static struct EffectObj death;death.ext.effect_26.timer=1;
 for(unsigned i=0;i<COUNT(g_QuadObjects);++i)g_QuadObjects[i].active=1;
 func_800BB364(&death);
 return !!effect_objects[0].active|((!!effect_objects[1].active)<<1)|((!effect_objects[2].active)<<2);
}
