#pragma once

/*
This file is part of Dweep Genesis.

Dweep Genesis is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

Dweep Genesis is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with Foobar. If not, see <https://www.gnu.org/licenses/>.
*/

#include <genesis.h>

#include "dweep_config.h"
#include "resources.h"
#include "palette_ctrl.h"

void TITLE_main()
{
    VDP_drawImageEx(BG_B, &title, TILE_ATTR_FULL(PAL1, 0, 0, 0, TILE_USER_INDEX), 0, 0, 0, DMA);
    VDP_drawBitmapEx(BG_A, &sgdk_logo, TILE_ATTR_FULL(PAL2, 0, 0, 0, TILE_USER_INDEX+title.tileset->numTile), 2, 2, FALSE);
    VDP_drawText(GAME_NAME, 1, 26);
    VDP_drawText(GAME_VERSION, 2+strlen(GAME_NAME), 26);

    PCTRL_set_line(1, title.palette->data, FALSE);
    PCTRL_set_line(2, sgdk_logo.palette->data, FALSE);

    PCTRL_fade_in(PAL_STD_FADE_DURATION);
    for(int i = 0; i < 40; i++)
        SYS_doVBlankProcess();

    PCTRL_fade_out(PAL_STD_FADE_DURATION);
    while(PAL_isDoingFade())
        SYS_doVBlankProcess();

}
