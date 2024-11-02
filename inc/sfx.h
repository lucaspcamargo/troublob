#pragma once

/*
This file is part of Blob Genesis.

Blob Genesis is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

Blob Genesis is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with Foobar. If not, see <https://www.gnu.org/licenses/>.
*/


#include <genesis.h>
#include "game_config.h"

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
