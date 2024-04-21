/*
This file is part of Dweep Genesis.

Dweep Genesis is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

Dweep Genesis is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with Foobar. If not, see <https://www.gnu.org/licenses/>.
*/

#include "plf_obj.h"
#include "resources.h"
#include "playfield.h"
#include "tools.h"
#include "sfx.h"

#include <genesis.h>

void PobjHandler_Hot(PobjData *data, enum PobjEventType evt, void* evt_arg)
{
    if(evt == POBJ_EVT_CREATED)
    {
        const PobjEvtCreatedArgs * const args = (PobjEvtCreatedArgs *) evt_arg;
        (void)args; // unused

        u16 tile_attrs = TILE_ATTR_FULL(PLF_theme_data_pal_line(PLF_THEME_HOT),
                                        0, 0, 0,
                                        PLF_theme_data_idx_table(PLF_THEME_HOT)[0][0]);
        PLF_plane_draw(TRUE, fix16ToInt(data->x), fix16ToInt(data->y), tile_attrs);
    }
    else if(evt == POBJ_EVT_STEPPED)
    {
        SFX_play(SFX_float);
    }
}

