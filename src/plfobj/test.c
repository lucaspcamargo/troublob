/*
This file is part of Dweep Genesis.

Dweep Genesis is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

Dweep Genesis is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with Foobar. If not, see <https://www.gnu.org/licenses/>.
*/

#include "plf_obj.h"
#include "playfield.h"
#include <genesis.h>

void PobjHandler_Test(PobjData *data, enum PobjEventType evt, void* evt_arg)
{
    if(evt == POBJ_EVT_CREATED)
    {
        VDP_drawTextEx(BG_B, "OO", TILE_ATTR(0,0,0,0), fix16ToInt(data->x)*2, fix16ToInt(data->y)*2, DMA);
        VDP_drawTextEx(BG_B, "OO", TILE_ATTR(0,0,0,0), fix16ToInt(data->x)*2, fix16ToInt(data->y)*2 + 1, DMA);
    }
}

