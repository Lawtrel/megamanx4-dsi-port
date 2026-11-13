/* Native port of sozud/mmx4 src/pc/placeholder.c:func_80028FEC.
 * AGPL-3.0. Original records, margins and object factories are preserved.
 * This creates native C objects; their AI/rendering integration is separate. */
#include "common.h"
#include "main/195B4.h"
#include "main/stage_object_record_tables.h"
#include "x4_stage_spawn.h"
extern struct StageObjectMarginData D_800F4334;
int x4_stage_spawn_visible(short left,short right,short top,short bottom,unsigned direction) {
    if((unsigned)engine_obj.stage>=13||(unsigned)engine_obj.substage>=2||
       direction>4||left>=right||top>=bottom||!SP_MENU_FRAMES||!D_801406A8)return -1;
    struct StageObjectRecord* record=D_800F43C8[(u8)engine_obj.stage][(u8)engine_obj.substage];
    if(!record)return -1;
    unsigned created=0;
    for(;record->object_type!=0xff;++record) {
        u8 flags=record->flags;
        if(flags&0x81)continue;
        s16 l=left,r=right,t=top,b=bottom;
        if(flags&0x70) {
            unsigned index=(flags>>4)-1;
            if(index>=5)return -1;
            u16 margin=D_800F4334.margins[index];
            if(direction==1){l+=margin;r+=margin;}
            if(direction==2){l-=margin;r-=margin;}
            if(direction==3){t+=margin;b+=margin;}
            if(direction==4){t-=margin;b-=margin;}
        }
        if(l<record->x&&record->x<r&&t<record->y&&record->y<b) {
            if(record->object_type>=8)return -1;
            struct ObjectHeader* object=MakeObject(record->object_type);
            if(!object)continue;
            object->active=0x41;object->id=record->id;object->unk2=record->subtype;
            object->x_pos.i.hi=record->x;object->y_pos.i.hi=record->y;object->backref=record;
            if(record->object_type==0) {
                struct MainObj* main=(struct MainObj*)object;
                u8 index=func_8002938C(record->id);
                main->unk40=D_801406A8[index]>>7;
                main->unk42=SOME_COORDINATE_CONVERSION(index);
                main->sprite_frames=(u8*)SP_MENU_FRAMES+SP_MENU_FRAMES[index];
            }
            record->flags|=1;++created;
        }
    }
    return created;
}
