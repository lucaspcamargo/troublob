/*
This file is part of Blob Genesis.

Blob Genesis is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

Blob Genesis is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with Foobar. If not, see <https://www.gnu.org/licenses/>.
*/

#include "registry.h"
#include "resources.h"
#include "tools.h"

#define PCTRL_NOP { PCTRL_OP_NOOP, 0, 0, 0 }

/**
 * Names of first 10 lvls
 * deflection
 * spin doctor
 * house of mirrors
 * bombing alley
 * Dexterity
 * Lasermania
 * Phoenix
 * Squish Time
 * Dweeps on Ice
 * Badda Bing Badda Boom
 */


const RGST_lvl RGST_levels[] = {
    {
        "Test Map\0                      ",
        &map_0_a,
        &map_0_b,
        &tset_0,
        &pal_tset_0,
        map_0_o,
        sizeof(map_0_o)/sizeof(void*),
        map_0_a_alloc,
        bgm_stage_3,
        {
            {PCTRL_OP_CYCLE, 16*PAL_LINE_BG_0 + 30, 2, 0x03},
            PCTRL_NOP,
            PCTRL_NOP,
            PCTRL_NOP
        },
        {
            TOOL_PLACE_MIRROR_RIGHT_UP,
            TOOL_PLACE_MIRROR_RIGHT_UP,
            TOOL_PLACE_MIRROR_RIGHT_DOWN,
            TOOL_PLACE_MIRROR_RIGHT_DOWN,
            TOOL_PLACE_BOMB,
            TOOL_PLACE_BOMB,
            TOOL_PLACE_BOMB,
            TOOL_PLACE_BOMB,
            TOOL_PLACE_BOMB,
        }
    }/*,
    {
        "Deflection\0                    ",
        &map_1_a,
        &map_1_b,
        &tset_1,
        &pal_tset_1,
        map_1_o,
        sizeof(map_1_o)/sizeof(void*),
        map_1_a_alloc,
        bgm_stage_1,
        {
            {PCTRL_OP_CYCLE, 16*PAL_LINE_BG_0 + 30, 2, 0x03},
            {PCTRL_OP_CYCLE, 16*PAL_LINE_BG_1 + 1,  4, 0x07},
            {PCTRL_OP_CYCLE, 16*PAL_LINE_BG_0 + 11, 2, 0x07},
            PCTRL_NOP
        },
        {
            TOOL_PLACE_MIRROR_RIGHT_UP,
            TOOL_PLACE_MIRROR_RIGHT_UP,
            TOOL_PLACE_MIRROR_RIGHT_UP,
            TOOL_PLACE_MIRROR_RIGHT_DOWN,
            TOOL_PLACE_MIRROR_RIGHT_DOWN,
            TOOL_PLACE_MIRROR_RIGHT_DOWN,
            TOOL_NONE,
            TOOL_NONE,
            TOOL_NONE
        }
    },
    {
        "Spin Doctor\0                   ",
        &map_2_a,
        &map_2_b,
        &tset_2,
        &pal_tset_2,
        map_2_o,
        sizeof(map_2_o)/sizeof(void*),
        map_2_a_alloc,
        bgm_stage_2,
        {
            {PCTRL_OP_CYCLE, 16*PAL_LINE_BG_0 + 30, 2, 0x03},
            PCTRL_NOP,
            PCTRL_NOP,
            PCTRL_NOP
        },
        {
            TOOL_ROTATE_CW,
            TOOL_ROTATE_CW,
            TOOL_ROTATE_CCW,
            TOOL_ROTATE_CCW,
            TOOL_NONE,
            TOOL_NONE,
            TOOL_NONE,
            TOOL_NONE,
            TOOL_NONE
        }
    },
    {
        "House of Mirrors\0              ",
        &map_3_a,
        &map_3_b,
        &tset_3,
        &pal_tset_3,
        map_3_o,
        sizeof(map_3_o)/sizeof(void*),
        map_3_a_alloc,
        bgm_stage_3,
        {
            {PCTRL_OP_CYCLE, 16*PAL_LINE_BG_0 + 30, 2, 0x03},
            PCTRL_NOP,
            PCTRL_NOP,
            PCTRL_NOP
        },
        {
            TOOL_NONE,
            TOOL_NONE,
            TOOL_NONE,
            TOOL_NONE,
            TOOL_NONE,
            TOOL_NONE,
            TOOL_NONE,
            TOOL_NONE,
            TOOL_NONE
        }
    },
    {
        "Bombing Alley\0                 ",
        &map_4_a,
        &map_4_b,
        &tset_4,
        &pal_tset_4,
        map_4_o,
        sizeof(map_4_o)/sizeof(void*),
        map_4_a_alloc,
        bgm_stage_4,
        {
            {PCTRL_OP_CYCLE, 16*PAL_LINE_BG_0 + 30, 2, 0x03},
            PCTRL_NOP,
            PCTRL_NOP,
            PCTRL_NOP
        },
        {
            TOOL_NONE,
            TOOL_NONE,
            TOOL_NONE,
            TOOL_NONE,
            TOOL_NONE,
            TOOL_NONE,
            TOOL_NONE,
            TOOL_NONE,
            TOOL_NONE
        }
    },
    {
        "Dexterity\0                     ",
        &map_5_a,
        &map_5_b,
        &tset_5,
        &pal_tset_5,
        map_5_o,
        sizeof(map_5_o)/sizeof(void*),
        map_5_a_alloc,
        bgm_stage_5,
        {
            {PCTRL_OP_CYCLE, 16*PAL_LINE_BG_0 + 30, 2, 0x03},
            PCTRL_NOP,
            PCTRL_NOP,
            PCTRL_NOP
        },
        {
            TOOL_NONE,
            TOOL_NONE,
            TOOL_NONE,
            TOOL_NONE,
            TOOL_NONE,
            TOOL_NONE,
            TOOL_NONE,
            TOOL_NONE,
            TOOL_NONE
        }
    },
    {
        "Lasermania\0                    ",
        &map_6_a,
        &map_6_b,
        &tset_1,
        &pal_tset_1,
        map_6_o,
        sizeof(map_6_o)/sizeof(void*),
        map_6_a_alloc,
        bgm_stage_1,
        {
            {PCTRL_OP_CYCLE, 16*PAL_LINE_BG_0 + 30, 2, 0x03},
            PCTRL_NOP,
            PCTRL_NOP,
            PCTRL_NOP
        },
        {
            TOOL_NONE,
            TOOL_NONE,
            TOOL_NONE,
            TOOL_NONE,
            TOOL_NONE,
            TOOL_NONE,
            TOOL_NONE,
            TOOL_NONE,
            TOOL_NONE
        }
    },
    {
        "Phoenix\0                       ",
        &map_7_a,
        &map_7_b,
        &tset_2,
        &pal_tset_2,
        map_7_o,
        sizeof(map_7_o)/sizeof(void*),
        map_7_a_alloc,
        bgm_stage_2,
        {
            {PCTRL_OP_CYCLE, 16*PAL_LINE_BG_0 + 30, 2, 0x03},
            PCTRL_NOP,
            PCTRL_NOP,
            PCTRL_NOP
        },
        {
            TOOL_NONE,
            TOOL_NONE,
            TOOL_NONE,
            TOOL_NONE,
            TOOL_NONE,
            TOOL_NONE,
            TOOL_NONE,
            TOOL_NONE,
            TOOL_NONE
        }
    },
    {
        "Squish Time\0                   ",
        &map_8_a,
        &map_8_b,
        &tset_3,
        &pal_tset_3,
        map_8_o,
        sizeof(map_8_o)/sizeof(void*),
        map_8_a_alloc,
        bgm_stage_3,
        {
            {PCTRL_OP_CYCLE, 16*PAL_LINE_BG_0 + 30, 2, 0x03},
            PCTRL_NOP,
            PCTRL_NOP,
            PCTRL_NOP
        },
        {
            TOOL_NONE,
            TOOL_NONE,
            TOOL_NONE,
            TOOL_NONE,
            TOOL_NONE,
            TOOL_NONE,
            TOOL_NONE,
            TOOL_NONE,
            TOOL_NONE
        }
    },
    {
        "Dweeps on Ice\0                 ",
        &map_9_a,
        &map_9_b,
        &tset_4,
        &pal_tset_4,
        map_9_o,
        sizeof(map_9_o)/sizeof(void*),
        map_9_a_alloc,
        bgm_stage_4,
        {
            {PCTRL_OP_CYCLE, 16*PAL_LINE_BG_0 + 30, 2, 0x03},
            PCTRL_NOP,
            PCTRL_NOP,
            PCTRL_NOP
        },
        {
            TOOL_NONE,
            TOOL_NONE,
            TOOL_NONE,
            TOOL_NONE,
            TOOL_NONE,
            TOOL_NONE,
            TOOL_NONE,
            TOOL_NONE,
            TOOL_NONE
        }
    },
    {
        "Badda Bing Badda Boom\0         ",
        &map_10_a,
        &map_10_b,
        &tset_5,
        &pal_tset_5,
        map_10_o,
        sizeof(map_10_o)/sizeof(void*),
        map_10_a_alloc,
        bgm_stage_5,
        {
            {PCTRL_OP_CYCLE, 16*PAL_LINE_BG_0 + 30, 2, 0x03},
            PCTRL_NOP,
            PCTRL_NOP,
            PCTRL_NOP
        },
        {
            TOOL_NONE,
            TOOL_NONE,
            TOOL_NONE,
            TOOL_NONE,
            TOOL_NONE,
            TOOL_NONE,
            TOOL_NONE,
            TOOL_NONE,
            TOOL_NONE
        }
    }*/
};
const uint16_t RGST_lvl_count = sizeof(RGST_levels)/sizeof(RGST_lvl);


const void* RGST_bgms[] = {
    bgm_title,
    bgm_stage_1,
    bgm_stage_2,
    bgm_stage_3,
    bgm_stage_4,
    bgm_stage_5,
    bgm_victory,
    bgm_defeat
};
const uint16_t RGST_bgm_count = sizeof(RGST_bgms)/sizeof(void*);
