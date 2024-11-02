#pragma once

/*
This file is part of Blob Genesis.

Blob Genesis is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

Blob Genesis is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with Foobar. If not, see <https://www.gnu.org/licenses/>.
*/


#include <genesis.h>
#include "palette_ctrl.h"
#include "tools.h"

#define RGST_PCTRL_OP_MAX 4

// a level in the game
// specifies where all the data is
// and other metadata
typedef struct RGST_lvl_st {
    const char name[32];
    const MapDefinition *bg_a_map;
    const MapDefinition *bg_b_map;
    const TileSet *bg_tileset;
    const Palette *bg_tileset_pal;
    const void ** const obj_map;
    const u16 obj_count;
    const u8 * bg_a_allocation;
    const u8 * bgm_xgm;
    PalCtrlOperatorDescriptor pal_ops[RGST_PCTRL_OP_MAX];
    const enum ToolId tool_inventory[9];
} RGST_lvl;

extern const RGST_lvl RGST_levels[];
extern const uint16_t RGST_lvl_count;

extern const void* RGST_bgms[];
extern const uint16_t RGST_bgm_count;
