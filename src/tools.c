#include "tools.h"
#include "playfield.h"
#include "player.h"
#include "input.h"
#include "resources.h"

bool TOOL_get_gfx(enum ToolId tool, u16 *out_frame, bool *out_flip_h, bool *out_flip_v)
{

    bool ret = FALSE;
    switch(tool)
    {
        case TOOL_MOVE:
            (*out_frame) = 0;
            ret = TRUE;
            break;
        case TOOL_PLACE_MIRROR_LEFT_DOWN:
            (*out_flip_h) = TRUE;
        case TOOL_PLACE_MIRROR_LEFT_UP:
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
        ret->prev_sprite = NULL;
        return;
    }

    ret->prev_sprite = NULL;
    ret->prev_flip_h = FALSE;
    ret->prev_anim = 0;
    ret->prev_pal_line = PAL_LINE_SPR_A;
    bool solid = t->attrs & PLF_ATTR_SOLID;
    bool has_obj = t->pobj != NULL;
    bool is_player = PLR_curr_tile_x() == plf_x && PLR_curr_tile_y() == plf_y;

    switch(tool)
    {
        case TOOL_MOVE:
            if (solid)
                goto no;
            ret->can_use = TRUE;
            ret->cursor = INPUT_CURSOR_MOVE;
            return;
        case TOOL_PLACE_MIRROR_LEFT_DOWN:
            ret->prev_flip_h = TRUE;
        case TOOL_PLACE_MIRROR_LEFT_UP:
            ret->prev_sprite = &spr_mirror;
            goto place_obj;
        case TOOL_PLACE_LASER_DOWN:
        case TOOL_PLACE_LASER_UP:
        case TOOL_PLACE_LASER_LEFT:
        case TOOL_PLACE_LASER_RIGHT:
        case TOOL_PLACE_FAN_DOWN:
        case TOOL_PLACE_FAN_UP:
        case TOOL_PLACE_FAN_LEFT:
        case TOOL_PLACE_FAN_RIGHT:
        case TOOL_PLACE_BOMB:
            ret->prev_sprite = &spr_bomb;
            goto place_obj;

        case TOOL_ROTATE_CCW:
        case TOOL_ROTATE_CW:
            // TODO obj query
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
    if(has_obj || solid)
        goto no;
    ret->can_use = TRUE;
    ret->cursor = INPUT_CURSOR_NORMAL;
}
