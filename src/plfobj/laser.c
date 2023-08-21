#include "plf_obj.h"
#include "resources.h"
#include "playfield.h"

#include <genesis.h>

typedef struct {
    Sprite * spr;
    u16 dir;
} PobjLaserExtraData;

void PobjHandler_Laser(PobjData *data, enum PobjEventType evt, void* evt_arg)
{
    PobjLaserExtraData * const extraData  = (PobjLaserExtraData*) &data->extra;
    if(evt == POBJ_EVT_CREATED)
    {
        const PobjEvtCreatedArgs * const args = (PobjEvtCreatedArgs *) evt_arg;
        ((PlfTile*)args->plftile)->attrs |= PLF_ATTR_SOLID;
        extraData->dir = args->subtype;

        extraData->spr = SPR_addSprite(&spr_laser_cannon, fix16ToInt(data->x)*16, fix16ToInt(data->y)*16 - 8, 0);
        SPR_setPalette(extraData->spr, PAL1);
        SPR_setAnim(extraData->spr, args->subtype);
        SPR_setDepth(extraData->spr, PLF_get_sprite_depth(data->x, data->y));
        // FIXME maybe world is not built yet, watch out
        PLF_laser_put(fix16ToInt(data->x), fix16ToInt(data->y), extraData->dir);
    }
}
