#pragma once

/*
This file is part of Blob Genesis.

Blob Genesis is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

Blob Genesis is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with Foobar. If not, see <https://www.gnu.org/licenses/>.
*/

#include "game_config.h"
#include <genesis.h>

enum ToolId {
    TOOL_NONE,
    TOOL_MOVE,
    TOOL_PLACE_MIRROR_RIGHT_DOWN,
    TOOL_PLACE_MIRROR_RIGHT_UP,
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
    const SpriteDefinition *preview_sprite;
    u8 preview_pal_line;
    bool preview_flip_h;
    u8 preview_anim;
} ToolQuery;

bool TOOL_get_gfx(enum ToolId tool, u16 *out_frame, bool *out_flip_h, bool *out_flip_v);
void TOOL_query(enum ToolId tool, u16 plf_x, u16 plf_y, ToolQuery *ret);
void TOOL_exec(enum ToolId tool, u16 plf_x, u16 plf_y);
