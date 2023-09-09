#include "plf_obj.h"
#include "resources.h"
#include "playfield.h"
#include "tools.h"
#include "sfx.h"

#include <genesis.h>

void PobjHandler_Hot(PobjData *data, enum PobjEventType evt, void* evt_arg)
{
    if(evt == POBJ_EVT_CREATED)
    {
        const PobjEvtCreatedArgs * const args = (PobjEvtCreatedArgs *) evt_arg;
        (void)args; // unused

        u16 tile_attrs = TILE_ATTR_FULL(PLF_theme_data_pal_line(PLF_THEME_HOT),
                                        0, 0, 0,
                                        PLF_theme_data_idx_table(PLF_THEME_HOT)[0][0]);
        PLF_plane_draw(TRUE, fix16ToInt(data->x), fix16ToInt(data->y), tile_attrs);
    }
    else if(evt == POBJ_EVT_STEPPED)
    {
        SFX_play(SFX_float);
    }
}

