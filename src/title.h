#pragma once
#include <genesis.h>

#include "dweep_config.h"
#include "resources.h"
#include "palette_ctrl.h"

void TITLE_main()
{
    PCTRL_set_source(1, title.palette->data, FALSE);
	PCTRL_fade_in(FADE_DUR);
	VDP_drawImageEx(BG_B, &title, TILE_ATTR_FULL(PAL1, 0, 0, 0, TILE_USER_INDEX), 0, 0, 0, DMA);
	VDP_drawText(GAME_NAME, 1, 26);
	VDP_drawText(GAME_VERSION, 2+strlen(GAME_NAME), 26);
	SYS_doVBlankProcess();

	XGM_setPCM(64, sfx_short, sizeof(sfx_short));
	XGM_setPCM(65, sfx_boom, sizeof(sfx_boom));

	XGM_startPlayPCM(64, 1, SOUND_PCM_CH1);
    SYS_doVBlankProcess();
	SYS_doVBlankProcess();

	while(XGM_isPlayingPCM(SOUND_PCM_CH1_MSK)) {
		SYS_doVBlankProcess();
	}

	XGM_startPlayPCM(65, 1, SOUND_PCM_CH2);
	SYS_doVBlankProcess();

	while(XGM_isPlayingPCM(SOUND_PCM_CH2_MSK)) {
		SYS_doVBlankProcess();
	}

	PCTRL_fade_out(FADE_DUR);
	while(PAL_isDoingFade())
		SYS_doVBlankProcess();

}
