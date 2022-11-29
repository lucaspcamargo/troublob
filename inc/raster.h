#pragma once

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
