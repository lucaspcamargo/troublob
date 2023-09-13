#include "tools.h"
#include "playfield.h"
#include "player.h"
#include "input.h"
#include "resources.h"
#include "plf_obj.h"
#include "sfx.h"

bool TOOL_get_gfx(enum ToolId tool, u16 *out_frame, bool *out_flip_h, bool *out_flip_v)
{

    bool ret = FALSE;
    switch(tool)
    {
        case TOOL_MOVE:
            (*out_frame) = 0;
            ret = TRUE;
            break;
        case TOOL_PLACE_MIRROR_RIGHT_DOWN:
            (*out_flip_h) = TRUE;
        case TOOL_PLACE_MIRROR_RIGHT_UP:
            (*out_frame) = 1;
            ret = TRUE;
            break;
        case TOOL_PLACE_LASER_DOWN:
            (*out_flip_v) = TRUE;
        case TOOL_PLACE_LASER_UP:
            (*out_frame) = 6;
            ret = TRUE;
            break;
        case TOOL_PLACE_LASER_LEFT:
            (*out_flip_h) = TRUE;
        case TOOL_PLACE_LASER_RIGHT:
            (*out_frame) = 5;
            ret = TRUE;
            break;
        case TOOL_PLACE_FAN_DOWN:
            (*out_flip_v) = TRUE;
        case TOOL_PLACE_FAN_UP:
            (*out_frame) = 8;
            ret = TRUE;
            break;
        case TOOL_PLACE_FAN_LEFT:
            (*out_flip_h) = TRUE;
        case TOOL_PLACE_FAN_RIGHT:
            (*out_frame) = 7;
            ret = TRUE;
            break;
        case TOOL_HAMMER:
            (*out_frame) = 2;
            ret = TRUE;
            break;
        case TOOL_ROTATE_CCW:
            (*out_flip_h) = TRUE;
        case TOOL_ROTATE_CW:
            (*out_frame) = 9;
            ret = TRUE;
            break;
        case TOOL_TORCH:
            (*out_frame) = 3;
            ret = TRUE;
            break;
        case TOOL_BUCKET:
            (*out_frame) = 4;
            ret = TRUE;
            break;
        case TOOL_PLACE_BOMB:
            (*out_frame) = 10;
            ret = TRUE;
            break;
        default:
            break;
    }
    return ret;
}


void TOOL_query(enum ToolId tool, u16 plf_x, u16 plf_y, ToolQuery *ret)
{
    const PlfTile * t = PLF_get_tile_safe(plf_x, plf_y);
    if (!t)
    {
        ret->can_use = FALSE;
        ret->cursor = INPUT_CURSOR_NORMAL;
        ret->preview_sprite = NULL;
        return;
    }

    ret->preview_sprite = NULL;
    ret->preview_flip_h = FALSE;
    ret->preview_anim = 0;
    ret->preview_pal_line = PAL_LINE_SPR_A;

    const bool solid_or_hole = t->attrs & (PLF_ATTR_PLAYER_SOLID|PLF_ATTR_HOLE);
    const bool has_obj = t->pobj != NULL;
    const bool is_player = PLR_curr_tile_x() == plf_x && PLR_curr_tile_y() == plf_y;

    switch(tool)
    {
        case TOOL_MOVE:
            if (solid_or_hole)
                goto no;
            ret->can_use = TRUE;
            ret->cursor = INPUT_CURSOR_MOVE;
            return;
        case TOOL_PLACE_MIRROR_RIGHT_DOWN:
            ret->preview_flip_h = TRUE;
        case TOOL_PLACE_MIRROR_RIGHT_UP:
            ret->preview_sprite = &spr_mirror;
            goto place_obj;
        case TOOL_PLACE_LASER_RIGHT:
            ret->preview_sprite = &spr_laser_cannon;
            ret->preview_anim = 0;
            goto place_obj;
        case TOOL_PLACE_LASER_LEFT:
            ret->preview_sprite = &spr_laser_cannon;
            ret->preview_anim = 1;
            goto place_obj;
        case TOOL_PLACE_LASER_UP:
            ret->preview_sprite = &spr_laser_cannon;
            ret->preview_anim = 2;
            goto place_obj;
        case TOOL_PLACE_LASER_DOWN:
            ret->preview_sprite = &spr_laser_cannon;
            ret->preview_anim = 3;
            goto place_obj;
        case TOOL_PLACE_FAN_DOWN:
        case TOOL_PLACE_FAN_UP:
        case TOOL_PLACE_FAN_LEFT:
        case TOOL_PLACE_FAN_RIGHT:
        case TOOL_PLACE_BOMB:
            ret->preview_sprite = &spr_bomb;
            goto place_obj;

        case TOOL_HAMMER:
        if(is_player)
        {
            // TODO unfloat?
        }
        case TOOL_ROTATE_CCW:
        case TOOL_ROTATE_CW:
            if(t->pobj)
            {
                PobjEvtToolQueryArgs args = {tool, FALSE, INPUT_CURSOR_NO};
                Pobj_event(Pobj_get_data(t->pobj), POBJ_EVT_TOOL_QUERY, &args);
                ret->can_use = args.out_can_use;
                ret->cursor = args.out_cursor;
                return;
            }
            goto no;
        case TOOL_TORCH:
            if(is_player)
            {
                ret->can_use = TRUE;
                // TODO is player wet?
                ret->cursor = INPUT_CURSOR_DEATH;
                return;
            }
            // TODO obj query
            break;
        case TOOL_BUCKET:
            if(is_player)
            {
                ret->can_use = TRUE;
                ret->cursor = INPUT_CURSOR_WATER;
                return;
            }
            // TODO obj query
            goto no;
        default:
            break;
    }

no:
    ret->can_use = FALSE;
    ret->cursor = INPUT_CURSOR_NO;
    return;

place_obj:
    if(has_obj || solid_or_hole || is_player)
        goto no;
    ret->can_use = TRUE;
    ret->cursor = INPUT_CURSOR_NORMAL;
}


void TOOL_exec(enum ToolId tool, u16 plf_x, u16 plf_y)
{
    const PlfTile * t = PLF_get_tile_safe(plf_x, plf_y);
    if (!t)
        return;

    const bool is_player = PLR_curr_tile_x() == plf_x && PLR_curr_tile_y() == plf_y;

    u16 create_type = POBJ_TYPE_COUNT;
    u16 create_subtype = 0;

    switch(tool)
    {
        case TOOL_MOVE:
            if(t->attrs & PLF_ATTR_PLAYER_SOLID)
                SFX_play(SFX_no);
            else
                PLR_goto(plf_x, plf_y);
            break;

        case TOOL_PLACE_MIRROR_RIGHT_DOWN:
            create_subtype = 1;
        case TOOL_PLACE_MIRROR_RIGHT_UP:
            create_type = POBJ_TYPE_MIRROR;
            goto place_obj;

        case TOOL_PLACE_LASER_RIGHT:
            create_type = POBJ_TYPE_LASER;
            create_subtype = 0;
            goto place_obj;
        case TOOL_PLACE_LASER_LEFT:
            create_type = POBJ_TYPE_LASER;
            create_subtype = 1;
            goto place_obj;
        case TOOL_PLACE_LASER_UP:
            create_type = POBJ_TYPE_LASER;
            create_subtype = 2;
            goto place_obj;
        case TOOL_PLACE_LASER_DOWN:
            create_type = POBJ_TYPE_LASER;
            create_subtype = 3;
            goto place_obj;
        case TOOL_PLACE_BOMB:
            create_type = POBJ_TYPE_BOMB;
            goto place_obj;

        case TOOL_HAMMER:
        case TOOL_TORCH:
            if(is_player)
            {
                // TODO player logic here
            }
        case TOOL_ROTATE_CCW:
        case TOOL_ROTATE_CW:
            if(t->pobj)
            {
                PobjEvtToolArgs args = {tool};
                Pobj_event(Pobj_get_data(t->pobj), POBJ_EVT_TOOL, &args);
            }
            return;
        default:
            break;
    }
    return;

place_obj:
    if(create_type == POBJ_TYPE_COUNT)
        return;
    PLF_obj_create(create_type, create_subtype, plf_x, plf_y);
    SFX_play(SFX_dull);
}
