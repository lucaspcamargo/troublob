/*
This file is part of Dweep Genesis.

Dweep Genesis is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

Dweep Genesis is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with Foobar. If not, see <https://www.gnu.org/licenses/>.
*/

#include "plf_obj.h"
#include "resources.h"
#include "playfield.h"
#include "tools.h"
#include "sfx.h"
#include "input.h"

#include <genesis.h>

#define TILE_BITS (PLF_ATTR_PLAYER_SOLID | PLF_ATTR_DANGER)
#define BOMB_FUSE_FRAMES 180
#define BOMB_FUSE_FRAMES_SHORT 30 // damage from another bomb
#define BOMB_TIMER_NONE 0xff
#define BOMB_TIMER_NONE_PLANE_A 0xfd // use this as sentinel for plane A, for now this means only unlit bombs can go there
#define BOMB_TIMER_EXPLODING 0xfe
typedef struct {
    Sprite * spr;
    u8 fuse_timer;
    u8 explode_timer;
} PobjBombExtraData;

void _Pobj_Bomb_revert_to_sprite(PobjData *data, PobjBombExtraData *extraData)
{
    if(extraData->spr || extraData->fuse_timer != BOMB_TIMER_NONE_PLANE_A)
        return; // we have to ensure we dont have a sprite and are really in plane a

    const u16 int_x = fix16ToInt(data->x);
    const u16 int_y = fix16ToInt(data->y);

    PLF_plane_a_uncover(int_x, int_y, PLF_ATTR_PLANE_A_OBJ_GFX);
    PLF_plane_clear(FALSE, int_x, int_y);

    extraData->spr = SPR_addSprite(PLF_theme_data_sprite_def(PLF_THEME_BOMB),
                int_x*16, int_y*16, 0);
    if(extraData->spr)
    {
        SPR_setAutoTileUpload(extraData->spr, FALSE);
        SPR_setPalette(extraData->spr, PLF_theme_data_pal_line(PLF_THEME_BOMB));
        SPR_setAnim(extraData->spr, 0);
        SPR_setVRAMTileIndex(extraData->spr, PLF_theme_data_idx_table(PLF_THEME_BOMB)[0][0]);
        SPR_setDepth(extraData->spr, PLF_get_sprite_depth(data->x, data->y));
    }
    extraData->fuse_timer = BOMB_TIMER_NONE;
}

bool _Pobj_Bomb_try_move_to_plane_a(PobjData *data, PobjBombExtraData *extraData)
{
    const u16 int_x = fix16ToInt(data->x);
    const u16 int_y = fix16ToInt(data->y);

    if(extraData->fuse_timer != BOMB_TIMER_NONE || !PLF_plane_a_cover(int_x, int_y, PLF_ATTR_PLANE_A_OBJ_GFX))
    return FALSE;

    if(extraData->spr)
    {
        SPR_releaseSprite(extraData->spr);
        extraData->spr = NULL;
    }

    extraData->fuse_timer = BOMB_TIMER_NONE_PLANE_A;
    const u16 tile_attrs = TILE_ATTR_FULL(PLF_theme_data_pal_line(PLF_THEME_BOMB),
                                    0, 0, 0,
                                    PLF_theme_data_idx_table(PLF_THEME_BOMB)[0][0]);
    PLF_plane_draw(FALSE, int_x, int_y, tile_attrs);

    return TRUE;

}

void PobjHandler_Bomb(PobjData *data, enum PobjEventType evt, void* evt_arg)
{
    PobjBombExtraData * const extraData  = (PobjBombExtraData*) &data->extra;
    const u16 int_x = fix16ToInt(data->x);
    const u16 int_y = fix16ToInt(data->y);

    if(evt == POBJ_EVT_CREATED)
    {
        const PobjEvtCreatedArgs * const args = (PobjEvtCreatedArgs *) evt_arg;
        PlfTile* tile = args->plftile;
        tile->attrs |= TILE_BITS;

        const bool lit = tile->laser? TRUE : FALSE;

        extraData->fuse_timer = lit? BOMB_FUSE_FRAMES : BOMB_TIMER_NONE;
        if((!lit) && PLF_plane_a_cover(int_x, int_y, PLF_ATTR_PLANE_A_OBJ_GFX))
        {
            extraData->spr = NULL;
            extraData->fuse_timer = BOMB_TIMER_NONE_PLANE_A;
            const u16 tile_attrs = TILE_ATTR_FULL(PLF_theme_data_pal_line(PLF_THEME_BOMB),
                                            0, 0, 0,
                                            PLF_theme_data_idx_table(PLF_THEME_BOMB)[0][0]);
            PLF_plane_draw(FALSE, int_x, int_y, tile_attrs);
        }
        else extraData->spr = SPR_addSprite(PLF_theme_data_sprite_def(PLF_THEME_BOMB),
                                    int_x*16, int_y*16, 0);
        if(extraData->spr)
        {
            SPR_setAutoTileUpload(extraData->spr, FALSE);
            SPR_setPalette(extraData->spr, PLF_theme_data_pal_line(PLF_THEME_BOMB));
            SPR_setAnim(extraData->spr, 0);
            SPR_setVRAMTileIndex(extraData->spr, PLF_theme_data_idx_table(PLF_THEME_BOMB)[0][lit? 1 : 0]);
            SPR_setDepth(extraData->spr, PLF_get_sprite_depth(data->x, data->y));
        }
    }
    else if(evt == POBJ_EVT_FRAME)
    {
        if(extraData->fuse_timer <= BOMB_FUSE_FRAMES)
        {
            if(--extraData->fuse_timer == 0)
            {
                // explode
                extraData->fuse_timer = BOMB_TIMER_EXPLODING;
                SFX_play(SFX_boom);
                if(extraData->spr)
                {
                    SPR_setHFlip(extraData->spr, random()%2);
                    SPR_setVFlip(extraData->spr, random()%2);
                    SPR_setPalette(extraData->spr, PLF_theme_data_pal_line(PLF_THEME_EXPLOSION));
                    SPR_setDefinition(extraData->spr, PLF_theme_data_sprite_def(PLF_THEME_EXPLOSION));
                    SPR_setPosition(extraData->spr, fix16ToInt(data->x)*16-8, fix16ToInt(data->y)*16-8);
                    SPR_setVRAMTileIndex(extraData->spr, PLF_theme_data_idx_table(PLF_THEME_EXPLOSION)[0][0]);
                    SPR_setDepth(extraData->spr, SPR_MIN_DEPTH + 1);
                }
                s16 startX = fix16ToInt(data->x) - 1;
                s16 endX = fix16ToInt(data->x) + 1;
                s16 startY = fix16ToInt(data->y) - 1;
                s16 endY = fix16ToInt(data->y) + 1;
                for(s16 x = startX; x <= endX; x++)
                    for(s16 y = startY; y <= endY; y++)
                        PLF_obj_damage(POBJ_DAMAGE_BOMB, x, y);
            }
            else if(extraData->spr)
            {
                SPR_setVRAMTileIndex(extraData->spr, PLF_theme_data_idx_table(PLF_THEME_BOMB)[0][1 + extraData->fuse_timer/4%2]);
            }
        }
        else if(extraData->fuse_timer != BOMB_TIMER_NONE && extraData->fuse_timer != BOMB_TIMER_NONE_PLANE_A)
        {
            // exploding
            extraData->explode_timer ++;
            if(extraData->explode_timer < 12)
            {
                // update anim
                if(extraData->spr)
                {
                    SPR_setVRAMTileIndex(extraData->spr, PLF_theme_data_idx_table(PLF_THEME_EXPLOSION)[0][extraData->explode_timer/2]);
                }
            }
            else
            {
                // really gone
                if(extraData->spr)
                    SPR_releaseSprite(extraData->spr);
                PLF_obj_destroy(fix16ToInt(data->x), fix16ToInt(data->y), NULL);
                PLF_get_tile(fix16ToInt(data->x), fix16ToInt(data->y))->attrs &= ~TILE_BITS;
            }
        }
    }
    else if(evt == POBJ_EVT_DAMAGE)
    {
        if(extraData->fuse_timer == BOMB_TIMER_EXPLODING)
            return;
        const bool unlit = (extraData->fuse_timer == BOMB_TIMER_NONE || extraData->fuse_timer == BOMB_TIMER_NONE_PLANE_A);
        if(*((enum PobjDamageType*) evt_arg) == POBJ_DAMAGE_LASER && unlit)
        {
            SFX_play(SFX_fuse);
            _Pobj_Bomb_revert_to_sprite(data, extraData);
            extraData->fuse_timer = BOMB_FUSE_FRAMES;
            if(extraData->spr)
            {
                SPR_setAnim(extraData->spr, 0);
                SPR_setVRAMTileIndex(extraData->spr, PLF_theme_data_idx_table(PLF_THEME_BOMB)[0][1]);
            }
        }
        else if(*((enum PobjDamageType*) evt_arg) == POBJ_DAMAGE_BOMB)
        {
            if (unlit)
            {
                SFX_play(SFX_fuse);
                _Pobj_Bomb_revert_to_sprite(data, extraData);
                extraData->fuse_timer = BOMB_FUSE_FRAMES_SHORT;
                if(extraData->spr)
                {
                    SPR_setAnim(extraData->spr, 0);
                    SPR_setVRAMTileIndex(extraData->spr, PLF_theme_data_idx_table(PLF_THEME_BOMB)[0][1]);
                }
            } else if (extraData->fuse_timer <= BOMB_FUSE_FRAMES && extraData->fuse_timer > BOMB_FUSE_FRAMES_SHORT)
            {
                // explosion accelerates lit bomb
                extraData->fuse_timer = BOMB_FUSE_FRAMES_SHORT;
            }
        }
    }
    else if(evt == POBJ_EVT_TOOL_QUERY)
    {
        if(extraData->fuse_timer == BOMB_TIMER_EXPLODING)
            return;
        PobjEvtToolQueryArgs * const args = (PobjEvtToolQueryArgs *) evt_arg;
        if(args->tool_id == TOOL_TORCH && extraData->fuse_timer == BOMB_TIMER_NONE)
        {
            args->out_can_use = TRUE;
            args->out_cursor = INPUT_CURSOR_FIRE;
        }
        else if(args->tool_id == TOOL_BUCKET && extraData->fuse_timer != BOMB_TIMER_NONE)
        {
            args->out_can_use = TRUE;
            args->out_cursor = INPUT_CURSOR_WATER;
        }
    }
    else if(evt == POBJ_EVT_TOOL)
    {
        if(extraData->fuse_timer == BOMB_TIMER_EXPLODING)
            return;

        enum ToolId tool = ((PobjEvtToolArgs*) evt_arg)->tool_id;
        if(tool == TOOL_BUCKET)
        {
            SFX_play(SFX_water);
            PlfTile *t = PLF_get_tile_safe(fix16ToInt(data->x), fix16ToInt(data->y));
            if (!t)
                return; // what???
            if(t->laser)
            {
                extraData->fuse_timer = BOMB_FUSE_FRAMES;
            }
            else
            {
                extraData->fuse_timer = BOMB_TIMER_NONE;
                SPR_setVRAMTileIndex(extraData->spr, PLF_theme_data_idx_table(PLF_THEME_BOMB)[0][0]);
            }
        }
        else if(tool == TOOL_TORCH && extraData->fuse_timer == BOMB_TIMER_NONE)
        {
            // light up normally
            SFX_play(SFX_fuse);
            extraData->fuse_timer = BOMB_FUSE_FRAMES;
            if(extraData->spr)
            {
                SPR_setVRAMTileIndex(extraData->spr, PLF_theme_data_idx_table(PLF_THEME_BOMB)[0][1]);
            }
        }
    }
    else if(evt == POBJ_EVT_PLANE_A_INVITE)
    {
        _Pobj_Bomb_try_move_to_plane_a(data, extraData);
    }
    else if(evt == POBJ_EVT_PLANE_A_EVICT)
    {
        _Pobj_Bomb_revert_to_sprite(data, extraData);
    }
    else if(evt == POBJ_EVT_DESTROYED)
    {
        if(extraData->fuse_timer == BOMB_TIMER_NONE_PLANE_A)
        {
            PLF_plane_a_uncover(int_x, int_y, PLF_ATTR_PLANE_A_OBJ_GFX);
            PLF_plane_clear(FALSE, int_x, int_y);
        }

        if(extraData->spr)
            SPR_releaseSprite(extraData->spr);
    }
}
