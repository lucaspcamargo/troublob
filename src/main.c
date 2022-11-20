#include "dweep_config.h"

#include <genesis.h>
#include "resources.h"
#include "string.h"

#include "palette_ctrl.h"
#include "title.h"
#include "playfield.h"
#include "hud.h"
#include "raster.h"

int main() {

	// subsystem init
	SYS_doVBlankProcess();
	SPR_init();
	PCTRL_init();
	
	// show title
	TITLE_main();

	// BEGIN main section
	// init playfield and hud
	PLF_init();
	HUD_init();

	// sprite palette and test
	PCTRL_set_source(PAL_LINE_SPR_A, spr_dweep.palette->data, FALSE);
	Sprite *spr = SPR_addSprite(&spr_dweep, 0, 0, PAL_LINE_SPR_A<<TILE_ATTR_PALETTE_SFT);
	SPR_setAlwaysVisible(spr, 1);
	SPR_setPosition(spr, 0,0);

	//Start play the level's song
	XGM_startPlay(bgm_stage_4);

	//int framecounter = 0;

	//RASTER_enable(); // enable raster interrupts

	PCTRL_fade_in(FADE_DUR);

	while(1) {

		//framecounter++;
		//int angle = (framecounter * 2)%1024;
		//fix16 cam_cx = fix16Add(FIX16(160), fix16Mul(sinFix16(angle), FIX16(40)));
		//fix16 cam_cy = fix16Add(FIX16(96), fix16Mul(cosFix16(angle), FIX16(40)));
		//PLF_cam_to(cam_cx, cam_cy); spins camera around

		PLF_update_scroll(FALSE); // update playfield scroll (no force redraw)
		SPR_update(); // step sprite system
		//PCTRL_step(); // evaluate palettes for next frame
		SYS_doVBlankProcess();
	}


// END IMAGE TEST

	return 0;
}
