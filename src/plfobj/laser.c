/*
This file is part of Blob Genesis.

Blob Genesis is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

Blob Genesis is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with Foobar. If not, see <https://www.gnu.org/licenses/>.
*/

#include "plf_obj.h"
#include "resources.h"
#include "playfield.h"
#include "input.h"
#include "sfx.h"

#include <genesis.h>

#define TILE_BITS (PLF_ATTR_PLAYER_SOLID | PLF_ATTR_LASER_SOLID | PLF_ATTR_DANGER)
#define LASER_DESTROY_FRAMES 50
#define LASER_EXPLOSION_MARK 0x80
typedef struct {
    Sprite * spr;
    u8 dir;
    u8 timer_destroy;
} PobjLaserExtraData;


void PobjHandler_Laser(PobjData *data, enum PobjEventType evt, void* evt_arg)
{
    PobjLaserExtraData * const extraData  = (PobjLaserExtraData*) &data->extra;
    if(evt == POBJ_EVT_CREATED)
    {
        const PobjEvtCreatedArgs * const args = (PobjEvtCreatedArgs *) evt_arg;
        PlfTile * tile = (PlfTile*) args->plftile;
        tile->attrs |= TILE_BITS;
        extraData->dir = args->subtype;

        const u16 xint = fix16ToInt(data->x);
        const u16 yint = fix16ToInt(data->y);
        if(yint)
            PLF_plane_a_cover(xint, yint-1, PLF_ATTR_PLANE_A_KEEPOUT);

        if(!tile->laser)
        {
            extraData->spr = SPR_addSprite(PLF_theme_data_sprite_def(PLF_THEME_LASER_CANNON),
                                xint*16, yint*16 - 8, 0);
            if(extraData->spr)
            {
                SPR_setAutoTileUpload(extraData->spr, FALSE);
                SPR_setPalette(extraData->spr, PAL_LINE_SPR_A);
                SPR_setAnim(extraData->spr, extraData->dir);
                SPR_setVRAMTileIndex(extraData->spr, PLF_theme_data_idx_table(PLF_THEME_LASER_CANNON)[extraData->dir][0]);
                SPR_setDepth(extraData->spr, PLF_get_sprite_depth(data->x, data->y));
            }
        }
        else
        {
            SFX_play(SFX_short);
            extraData->timer_destroy = LASER_DESTROY_FRAMES;
            PLF_laser_recalc(xint, yint);
            if(extraData->spr)
            {
                SPR_releaseSprite(extraData->spr);
                extraData->spr = NULL;
            }

            extraData->spr = SPR_addSprite(&spr_laser_cannon_d,
                                            fix16ToInt(data->x)*16, fix16ToInt(data->y)*16 - 8,
                                            TILE_ATTR(PAL_LINE_BG_1, 0, 0, 0));
            if(extraData->spr)
            {
                SPR_setAutoTileUpload(extraData->spr, TRUE);
                SPR_setAnim(extraData->spr, extraData->dir);
            }
        }

        PLF_laser_put(fix16ToInt(data->x), fix16ToInt(data->y), extraData->dir);
    }
    else if(evt == POBJ_EVT_TOOL_QUERY)
    {
        if(extraData->timer_destroy)
            return;

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
        if(extraData->timer_destroy)
            return;

        const PobjEvtToolArgs * const args = (PobjEvtToolArgs *) evt_arg;
        if(args->tool_id == TOOL_ROTATE_CW || args->tool_id == TOOL_ROTATE_CCW)
        {
            extraData->dir = (args->tool_id == TOOL_ROTATE_CW? DIR_ROT_CW(extraData->dir) : DIR_ROT_CCW(extraData->dir)); // flip
            PLF_laser_recalc(fix16ToInt(data->x), fix16ToInt(data->y));
            PLF_laser_put(fix16ToInt(data->x), fix16ToInt(data->y), extraData->dir);
            if(extraData->spr)
                SPR_setVRAMTileIndex(extraData->spr, PLF_theme_data_idx_table(PLF_THEME_LASER_CANNON)[extraData->dir][0]);
            SFX_play(SFX_wrench);
        }
        else if(args->tool_id == TOOL_HAMMER)
        {
            SFX_play(SFX_short);
            extraData->timer_destroy = LASER_DESTROY_FRAMES;
            if(extraData->spr)
            {
                SPR_releaseSprite(extraData->spr);
                extraData->spr = SPR_addSprite(&spr_laser_cannon_d,
                                               fix16ToInt(data->x)*16, fix16ToInt(data->y)*16 - 8,
                                               TILE_ATTR(PAL_LINE_BG_1, 0, 0, 0));
                if(extraData->spr)
                {
                    SPR_setAutoTileUpload(extraData->spr, TRUE);
                    SPR_setAnim(extraData->spr, extraData->dir);
                }
            }
        }
    }
    else if(evt == POBJ_EVT_FRAME)
    {
        if(extraData->timer_destroy >= LASER_EXPLOSION_MARK)
        {
            // explosion now
            extraData->timer_destroy ++;
            if(extraData->timer_destroy < (LASER_EXPLOSION_MARK+12))
            {
                // update anim
                if(extraData->spr)
                {
                    SPR_setVRAMTileIndex(extraData->spr, PLF_theme_data_idx_table(PLF_THEME_EXPLOSION)[0][(extraData->timer_destroy&~LASER_EXPLOSION_MARK)/2]);
                }
            }
            else
            {
                PLF_obj_destroy(fix16ToInt(data->x), fix16ToInt(data->y), NULL);
                if(extraData->spr)
                    SPR_releaseSprite(extraData->spr);
            }
        }
        else if(extraData->timer_destroy && !(--(extraData->timer_destroy)))
        {
            SFX_play(SFX_boom);
            extraData->timer_destroy = LASER_EXPLOSION_MARK;
            if(extraData->spr)
            {
                SPR_setAutoTileUpload(extraData->spr, FALSE);
                SPR_setPalette(extraData->spr, PAL_LINE_SPR_A);
                SPR_setHFlip(extraData->spr, random()%2);
                SPR_setVFlip(extraData->spr, random()%2);
                SPR_setDefinition(extraData->spr, PLF_theme_data_sprite_def(PLF_THEME_EXPLOSION));
                SPR_setPosition(extraData->spr, fix16ToInt(data->x)*16-8, fix16ToInt(data->y)*16-8);
                SPR_setVRAMTileIndex(extraData->spr, PLF_theme_data_idx_table(PLF_THEME_EXPLOSION)[0][0]);
                SPR_setDepth(extraData->spr, SPR_MIN_DEPTH + 1);
            }
            PLF_get_tile(fix16ToInt(data->x), fix16ToInt(data->y))->attrs &= ~TILE_BITS;
            PLF_laser_recalc(fix16ToInt(data->x), fix16ToInt(data->y));

        }
        else if(extraData->spr && !extraData->timer_destroy)
            SPR_setVRAMTileIndex(extraData->spr, PLF_theme_data_idx_table(PLF_THEME_LASER_CANNON)[extraData->dir][*((u32*)evt_arg)/4 % 2]);
    }
    else if(evt == POBJ_EVT_DAMAGE)
    {
        if(extraData->timer_destroy)
            return;
        enum PobjDamageType damage = *((enum PobjDamageType*)evt_arg);
        if(damage == POBJ_DAMAGE_BOMB || damage == POBJ_DAMAGE_LASER)
        {
            SFX_play(SFX_short);
            extraData->timer_destroy = LASER_DESTROY_FRAMES;
            if(extraData->spr)
            {
                SPR_releaseSprite(extraData->spr);
                extraData->spr = SPR_addSprite(&spr_laser_cannon_d,
                                               fix16ToInt(data->x)*16, fix16ToInt(data->y)*16 - 8,
                                               TILE_ATTR(PAL_LINE_BG_1, 0, 0, 0));
                if(extraData->spr)
                {
                    SPR_setAutoTileUpload(extraData->spr, TRUE);
                    SPR_setAnim(extraData->spr, extraData->dir);
                }
            }
        }
    }
    else if(evt == POBJ_EVT_DESTROYED)
    {
        const u16 xint = fix16ToInt(data->x);
        const u16 yint = fix16ToInt(data->y);
        if(yint)
            PLF_plane_a_uncover(xint, yint-1, PLF_ATTR_PLANE_A_KEEPOUT);

        if(extraData->spr)
            SPR_releaseSprite(extraData->spr);
    }
}
