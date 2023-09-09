#include "plf_obj.h"
#include "resources.h"
#include "playfield.h"

#include <genesis.h>

typedef struct {
    Sprite * spr;
    u16 var;
} PobjMirrorExtraData;

void PobjHandler_Mirror(PobjData *data, enum PobjEventType evt, void* evt_arg)
{
    PobjMirrorExtraData * const extraData  = (PobjMirrorExtraData*) &data->extra;
    if(evt == POBJ_EVT_CREATED)
    {
        const PobjEvtCreatedArgs * const args = (PobjEvtCreatedArgs *) evt_arg;
        ((PlfTile*)args->plftile)->attrs |= (PLF_ATTR_PLAYER_SOLID|PLF_ATTR_DANGER);
        extraData->var = args->subtype;

        extraData->spr = SPR_addSprite(PLF_theme_data_sprite_def(PLF_THEME_MIRROR),
                                       fix16ToInt(data->x)*16, fix16ToInt(data->y)*16 - 8, 0);
        if(extraData->spr)
        {
            SPR_setAutoTileUpload(extraData->spr, FALSE);
            SPR_setPalette(extraData->spr, PAL_LINE_SPR_A);
            SPR_setAnim(extraData->spr, 0);
            SPR_setVRAMTileIndex(extraData->spr, PLF_theme_data_idx_table(PLF_THEME_MIRROR)[0][0]);
            SPR_setHFlip(extraData->spr,extraData->var? TRUE : FALSE);
            SPR_setDepth(extraData->spr, PLF_get_sprite_depth(data->x, data->y));
        }

        if(args->plftile && ((PlfTile*)args->plftile)->laser)
            PLF_laser_recalc(fix16ToInt(data->x), fix16ToInt(data->y));
    }
    else if(evt == POBJ_EVT_LASER_QUERY)
    {
         *((enum PobjLaserBehavior*)evt_arg) = (extraData->var? POBJ_LASER_REFLECT_RIGHT_DOWN : POBJ_LASER_REFLECT_RIGHT_UP);
    }
}
