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
            TOOL_NONE,
            TOOL_NONE,
            TOOL_NONE,
            TOOL_NONE,
            TOOL_NONE,
        }
    },
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
        &tset_1,
        &pal_tset_1,
        map_2_o,
        sizeof(map_2_o)/sizeof(void*),
        map_2_a_alloc,
        bgm_stage_4,
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
            TOOL_PLACE_BOMB,
            TOOL_NONE,
            TOOL_NONE
        }
    }
};
const uint16_t RGST_lvl_count = sizeof(RGST_levels)/sizeof(RGST_lvl);


const void* RGST_bgms[] = {
    bgm_stage_1,
    bgm_stage_2,
    bgm_stage_3,
    bgm_stage_4,
    bgm_stage_5,
    bgm_victory,
    bgm_defeat
};
const uint16_t RGST_bgm_count = sizeof(RGST_bgms)/sizeof(void*);
