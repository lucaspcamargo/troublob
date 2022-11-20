#pragma once

#include <genesis.h>

#include "dweep_config.h"
#include "palette_ctrl.h"

static u8 raster_line_ctr = 0;

HINTERRUPT_CALLBACK on_h_int(void)
{
	if(raster_line_ctr == RASTER_HUD_LINE)
	{
		PCTRL_hud_h_int();
		VDP_setVerticalScroll(BG_A, 0);
		VDP_setHorizontalScroll(BG_A, 0);
		VDP_setVerticalScroll(BG_B, 0);
		VDP_setHorizontalScroll(BG_B, 0);
	}
	else if(raster_line_ctr == RASTER_HUD_LINE_END)
    {
		PCTRL_hud_end_h_int();
    }
	raster_line_ctr ++;
}

void on_v_int(void)
{
	raster_line_ctr = 0;
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
