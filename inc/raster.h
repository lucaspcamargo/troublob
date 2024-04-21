#pragma once

/*
This file is part of Dweep Genesis.

Dweep Genesis is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

Dweep Genesis is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with Foobar. If not, see <https://www.gnu.org/licenses/>.
*/


#include <genesis.h>

#include "dweep_config.h"
#include "palette_ctrl.h"


HINTERRUPT_CALLBACK on_h_int(void)
{
}

void on_v_int(void)
{

}


void RASTER_enable()
{
    // setup beam callbacks
	SYS_disableInts();
    {
        VDP_setHIntCounter(0); // skip=0, means int every line
        VDP_setHInterrupt(1);  // enable
		SYS_setHIntCallback(&on_h_int);
        SYS_setVIntCallback(&on_v_int);
    }
    SYS_enableInts();
}
