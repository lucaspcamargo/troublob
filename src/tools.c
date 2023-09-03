#include "tools.h"


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
