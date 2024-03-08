#pragma once

#include <genesis.h>
#include "dweep_config.h"

enum SfxId {
    SFX_boom = SFX_USER_OFFSET,
    SFX_burn,
    SFX_crush,
    SFX_ding,
    SFX_dry,
    SFX_dull,
    SFX_float,
    SFX_freeze,
    SFX_fuse,
    SFX_glass,
    SFX_go1,
    SFX_go3,
    SFX_go4,
    SFX_mop,
    SFX_no,
    SFX_short,
    SFX_thaw,
    SFX_water,
    SFX_wrench,
    SFX_cachacella,
    SFX_END
};

void SFX_register_all();

#define SFX_play(ID) SFX_play_prio((ID),0)

void SFX_play_prio(u8 id, u8 prio);
