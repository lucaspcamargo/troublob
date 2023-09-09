#include "plf_obj.h"
#include "resources.h"
#include "playfield.h"
#include "tools.h"
#include "sfx.h"

#include <genesis.h>

#define TILE_BITS (PLF_ATTR_PLAYER_SOLID | PLF_ATTR_DANGER)

typedef struct {
    Sprite * spr;
} PobjBombExtraData;

void PobjHandler_Bomb(PobjData *data, enum PobjEventType evt, void* evt_arg)
{
    PobjBombExtraData * const extraData  = (PobjBombExtraData*) &data->extra;

    if(evt == POBJ_EVT_CREATED)
    {
        const PobjEvtCreatedArgs * const args = (PobjEvtCreatedArgs *) evt_arg;
        ((PlfTile*)args->plftile)->attrs |= TILE_BITS;

        extraData->spr = SPR_addSprite(PLF_theme_data_sprite_def(PLF_THEME_BOMB),
                                       fix16ToInt(data->x)*16, fix16ToInt(data->y)*16, 0);
        if(extraData->spr)
        {
            SPR_setAutoTileUpload(extraData->spr, FALSE);
            SPR_setPalette(extraData->spr, PAL_LINE_SPR_A);
            SPR_setAnim(extraData->spr, 0);
            SPR_setVRAMTileIndex(extraData->spr, PLF_theme_data_idx_table(PLF_THEME_BOMB)[0][0]);
            SPR_setDepth(extraData->spr, PLF_get_sprite_depth(data->x, data->y));
        }
    }
}
