#pragma once
#include "dweep_config.h"
#include <genesis.h>

enum ToolId {
    TOOL_NONE,
    TOOL_MOVE,
    TOOL_PLACE_MIRROR_LEFT_DOWN,
    TOOL_PLACE_MIRROR_LEFT_UP,
    TOOL_PLACE_LASER_UP,
    TOOL_PLACE_LASER_DOWN,
    TOOL_PLACE_LASER_LEFT,
    TOOL_PLACE_LASER_RIGHT,
    TOOL_PLACE_FAN_UP,
    TOOL_PLACE_FAN_DOWN,
    TOOL_PLACE_FAN_LEFT,
    TOOL_PLACE_FAN_RIGHT,
    TOOL_PLACE_BOMB,
    TOOL_HAMMER,
    TOOL_ROTATE_CW,
    TOOL_ROTATE_CCW,
    TOOL_TORCH,
    TOOL_BUCKET,
    TOOL_COUNT
} ENUM_PACK;

typedef struct ToolQuery_st
{
    bool can_use;
    u8 cursor;
    const SpriteDefinition *prev_sprite;
    u8 prev_pal_line;
    bool prev_flip_h;
    u8 prev_anim;
} ToolQuery;

bool TOOL_get_gfx(enum ToolId tool, u16 *out_frame, bool *out_flip_h, bool *out_flip_v);
void TOOL_query(enum ToolId tool, u16 plf_x, u16 plf_y, ToolQuery *ret);
void TOOL_exec(enum ToolId tool, u16 plf_x, u16 plf_y);
