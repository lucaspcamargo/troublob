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
#define BOMB_TIMER_EXPLODING 0xfe
typedef struct {
    Sprite * spr;
    u8 fuse_timer;
    u8 explode_timer;
} PobjBombExtraData;

void PobjHandler_Bomb(PobjData *data, enum PobjEventType evt, void* evt_arg)
{
    PobjBombExtraData * const extraData  = (PobjBombExtraData*) &data->extra;

    if(evt == POBJ_EVT_CREATED)
    {
        const PobjEvtCreatedArgs * const args = (PobjEvtCreatedArgs *) evt_arg;
        PlfTile* tile = args->plftile;
        tile->attrs |= TILE_BITS;

        if (tile->laser)
        {
            // lit
            extraData->fuse_timer = BOMB_FUSE_FRAMES;
            extraData->spr = SPR_addSprite(PLF_theme_data_sprite_def(PLF_THEME_BOMB),
                                        fix16ToInt(data->x)*16, fix16ToInt(data->y)*16, 0);
            if(extraData->spr)
            {
                SPR_setAutoTileUpload(extraData->spr, FALSE);
                SPR_setPalette(extraData->spr, PAL_LINE_SPR_A);
                SPR_setAnim(extraData->spr, 0);
                SPR_setVRAMTileIndex(extraData->spr, PLF_theme_data_idx_table(PLF_THEME_BOMB)[0][1]);
                SPR_setDepth(extraData->spr, PLF_get_sprite_depth(data->x, data->y));
            }
        }
        else
        {
            // not lit
            extraData->fuse_timer = BOMB_TIMER_NONE;
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
                const u32 framecounter = *((u32*)evt_arg);
                SPR_setVRAMTileIndex(extraData->spr, PLF_theme_data_idx_table(PLF_THEME_BOMB)[0][1 + extraData->fuse_timer/4%2]);
            }
        }
        else if(extraData->fuse_timer != BOMB_TIMER_NONE)
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
        if(*((enum PobjDamageType*) evt_arg) == POBJ_DAMAGE_LASER && extraData->fuse_timer == BOMB_TIMER_NONE)
        {
            SFX_play(SFX_fuse);
            extraData->fuse_timer = BOMB_FUSE_FRAMES;
            if(extraData->spr)
            {
                SPR_setAnim(extraData->spr, 0);
                SPR_setVRAMTileIndex(extraData->spr, PLF_theme_data_idx_table(PLF_THEME_BOMB)[0][1]);
            }
        }
        else if(*((enum PobjDamageType*) evt_arg) == POBJ_DAMAGE_BOMB)
        {
            if (extraData->fuse_timer == BOMB_TIMER_NONE)
            {
                SFX_play(SFX_fuse);
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
            args->out_cursor = INPUT_CURSOR_DEATH; // TODO flame cursor
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
}
