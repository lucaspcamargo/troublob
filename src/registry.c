#include "registry.h"
#include "resources.h"

const RGST_lvl RGST_levels[] = {
    {
        "TEST LEVEL\0                    ",
        &map_2_a,
        &map_2_b,
        &tset_1,
        &pal_tset_1,
        map_2_o,
        sizeof(map_2_o)/sizeof(void*)
    }
};
const uint16_t RGST_lvl_count = sizeof(RGST_levels)/sizeof(RGST_lvl);
