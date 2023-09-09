#include "plf_obj.h"
#include "resources.h"
#include "playfield.h"
#include "tools.h"
#include "hud.h"
#include "sfx.h"

#include <genesis.h>

typedef struct {
    enum ToolId tool_id;
} PobjItemExtraData;

void PobjHandler_ToolItem(PobjData *data, enum PobjEventType evt, void* evt_arg)
{
    PobjItemExtraData * const extraData  = (PobjItemExtraData*) &data->extra;
    if(evt == POBJ_EVT_CREATED)
    {
        const PobjEvtCreatedArgs * const args = (PobjEvtCreatedArgs *) evt_arg;
        extraData->tool_id = args->subtype;
        u16 frame = 0;
        bool flipH = FALSE;
        bool flipV = FALSE;
        if(TOOL_get_gfx(extraData->tool_id, &frame, &flipH, &flipV))
        {
            u16 tile_attrs = TILE_ATTR_FULL(PLF_theme_data_pal_line(PLF_THEME_TOOLS),
                                            0, flipV, flipH,
                                            PLF_theme_data_idx_table(PLF_THEME_TOOLS)[0][frame]);
            PLF_plane_draw(TRUE, fix16ToInt(data->x), fix16ToInt(data->y), tile_attrs);
        }
    }
    else if(evt == POBJ_EVT_STEPPED)
    {
        // player stepped on me
        if(HUD_inventory_push(extraData->tool_id))
        {
            PLF_plane_clear(TRUE, fix16ToInt(data->x), fix16ToInt(data->y));
            extraData->tool_id = TOOL_NONE;
            SFX_play(SFX_ding);
            PLF_obj_destroy(fix16ToInt(data->x), fix16ToInt(data->y), NULL);
        }
    }
}

