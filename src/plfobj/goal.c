#include "plf_obj.h"
#include "resources.h"
#include "playfield.h"

#include <genesis.h>

typedef struct {
    Sprite * baby;
} PobjGoalExtraData;

void PobjHandler_Goal(PobjData *data, enum PobjEventType evt, void* evt_arg)
{
    PobjGoalExtraData * const extraData  = (PobjGoalExtraData*) &data->extra;
    if(evt == POBJ_EVT_CREATED)
    {
        //const PobjEvtCreatedArgs * const args = (PobjEvtCreatedArgs *) evt_arg;
        u16 tile_attrs = TILE_ATTR_FULL(PLF_theme_data_pal_line(PLF_THEME_GOAL),
                                        0, 0, 0,
                                        PLF_theme_data_idx_table(PLF_THEME_GOAL)[0][0]);
        PLF_plane_draw(TRUE, fix16ToInt(data->x), fix16ToInt(data->y), tile_attrs);

        u8 baby_color = random()%3;
        u16 tile_attrs_b = TILE_ATTR_FULL(PLF_theme_data_pal_line(PLF_THEME_GOAL_BABIES),
                                            0, 0, 0, 0);
        extraData->baby = SPR_addSprite(PLF_theme_data_sprite_def(PLF_THEME_GOAL_BABIES),
                                          fix16ToInt(data->x)*16+4, fix16ToInt(data->y)*16+4,
                                          tile_attrs_b);
        SPR_setAutoTileUpload(extraData->baby, FALSE);
        SPR_setAnimAndFrame(extraData->baby, 0, baby_color);
        SPR_setVRAMTileIndex(extraData->baby, PLF_theme_data_idx_table(PLF_THEME_GOAL_BABIES)[0][baby_color]);
        SPR_setDepth(extraData->baby, PLF_get_sprite_depth(FIX16(data->x), FIX16(data->y)) - 1);
    }
    else if(evt == POBJ_EVT_STEPPED)
    {
        // TODO elaborate on this
        // change player state to victory or something
        // and reimpl this on playfield exec
        VDP_drawText("              ",13, 11);
        VDP_drawText("  VICTORY!!!  ",13, 12);
        VDP_drawText("              ",13, 13);
        XGM_setLoopNumber(0);
        XGM_startPlay(bgm_victory);
        SYS_doVBlankProcess();
        while(XGM_isPlaying())
            SYS_doVBlankProcess();
        XGM_setLoopNumber(-1);
    }
}
