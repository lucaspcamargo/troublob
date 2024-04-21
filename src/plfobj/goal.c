/*
This file is part of Dweep Genesis.

Dweep Genesis is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

Dweep Genesis is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with Foobar. If not, see <https://www.gnu.org/licenses/>.
*/

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
        if(extraData->baby)
        {
            SPR_setAutoTileUpload(extraData->baby, FALSE);
            SPR_setAnimAndFrame(extraData->baby, 0, baby_color);
            SPR_setVRAMTileIndex(extraData->baby, PLF_theme_data_idx_table(PLF_THEME_GOAL_BABIES)[0][baby_color]);
            SPR_setDepth(extraData->baby, PLF_get_sprite_depth(data->x, data->y) - 3);
        }
    }
    else if(evt == POBJ_EVT_FRAME)
    {
        if(extraData->baby)
        {
            const u32 framecounter = *((u32*) evt_arg);
            fix16 delta = sinFix16((framecounter*16)%1024);
            if(delta > 0)
                delta = fix16Neg(delta);
            SPR_setPosition(extraData->baby, fix16ToInt(data->x)*16+4, fix16ToInt(data->y)*16+3+(delta>>3));
        }
    }
    else if(evt == POBJ_EVT_STEPPED)
    {
        // TODO elaborate on this
        // change player state to victory or something
        // and reimpl this on playfield exec
        VDP_drawText("              ",13, 11);
        VDP_drawText("  VICTORY!!!  ",13, 12);
        VDP_drawText("              ",13, 13);
        XGM_stopPlay();
        XGM_setLoopNumber(0);
        XGM_startPlay(bgm_victory);
        SYS_doVBlankProcess();
        while(XGM_isPlaying())
            SYS_doVBlankProcess();
        XGM_setLoopNumber(-1);
    }
    else if(evt == POBJ_EVT_DESTROYED)
    {
        if(extraData->baby)
            SPR_releaseSprite(extraData->baby);
    }
}
