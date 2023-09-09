#include "plf_obj.h"
#include "resources.h"
#include "playfield.h"
#include "input.h"
#include "sfx.h"

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
    else if(evt == POBJ_EVT_TOOL_QUERY)
    {
        PobjEvtToolQueryArgs * const args = (PobjEvtToolQueryArgs *) evt_arg;
        if(args->tool_id == TOOL_ROTATE_CW || args->tool_id == TOOL_ROTATE_CCW)
        {
            args->out_can_use = TRUE;
            args->out_cursor = (args->tool_id == TOOL_ROTATE_CW? INPUT_CURSOR_ROTATE_CW : INPUT_CURSOR_ROTATE_CCW);
        }
        else if(args->tool_id == TOOL_HAMMER)
        {
            args->out_can_use = TRUE;
            args->out_cursor = INPUT_CURSOR_HAMMER;
        }
    }
    else if(evt == POBJ_EVT_TOOL)
    {
        const PobjEvtToolArgs * const args = (PobjEvtToolArgs *) evt_arg;
        if(args->tool_id == TOOL_ROTATE_CW || args->tool_id == TOOL_ROTATE_CCW)
        {
            extraData->var = !extraData->var; // flip
            if(extraData->spr)
                SPR_setHFlip(extraData->spr,extraData->var? TRUE : FALSE);
            PLF_laser_recalc(fix16ToInt(data->x), fix16ToInt(data->y));
            SFX_play(SFX_wrench);
        }
        else if(args->tool_id == TOOL_HAMMER)
        {
            // TODO destruction anim
            SFX_play(SFX_glass);
            if(extraData->spr)
                SPR_releaseSprite(extraData->spr);
            PLF_obj_destroy(fix16ToInt(data->x), fix16ToInt(data->y), NULL);
            PLF_laser_recalc(fix16ToInt(data->x), fix16ToInt(data->y));
        }
    }
}
