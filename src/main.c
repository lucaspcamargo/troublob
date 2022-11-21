#include "dweep_config.h"

#include <genesis.h>
#include "resources.h"
#include "string.h"

#include "palette_ctrl.h"
#include "title.h"
#include "playfield.h"
#include "hud.h"
#include "raster.h"
#include "mouse.h"
#include "sfx.h"




int main() {

	// subsystem init
	SYS_doVBlankProcess();
	SPR_init();
	PCTRL_init();
	MOUSE_init();
	SFX_register_all();

	// show title
	TITLE_main();

	// BEGIN main section
	// init playfield and hud
	PLF_init();
	HUD_init();

	// player
	PCTRL_set_source(PAL_LINE_SPR_A, spr_dweep.palette->data, FALSE);
	Sprite *spr_player = SPR_addSprite(&spr_dweep, 0, 0, PAL_LINE_SPR_A<<TILE_ATTR_PALETTE_SFT);
	Sprite *spr_player_shadow = SPR_addSprite(&spr_shadow, 0, 0, PAL_LINE_SPR_A<<TILE_ATTR_PALETTE_SFT);
	SPR_setPriority(spr_player_shadow, FALSE);
	SPR_setAlwaysVisible(spr_player, 1);
	fix16 player_pf_x = FIX16(3);
	fix16 player_pf_y = FIX16(2);
	fix16 player_pf_z = FIX16(0);
	fix16 dest_pf_x = player_pf_x;
	fix16 dest_pf_y = player_pf_y;

#define positionPlayer() SPR_setPosition(spr_player, \
	fix16ToRoundedInt(fix16Mul(player_pf_x, FIX16(16))), \
	fix16ToRoundedInt(fix16Sub(fix16Mul(player_pf_y, FIX16(16)), player_pf_z))-8+3)
#define positionShadow() SPR_setPosition(spr_player_shadow, \
	fix16ToRoundedInt(fix16Mul(player_pf_x, FIX16(16))), \
	fix16ToRoundedInt(fix16Mul(player_pf_y, FIX16(16)))+4+3)
#define scaleShadow() SPR_setFrame(spr_player_shadow, \
	((s16) (fix16Div(player_pf_z, FIX16(3)) >> FIX16_FRAC_BITS)) );

	positionPlayer();
	positionShadow();
	scaleShadow();

	//Start play the level's song
	XGM_startPlay(bgm_stage_1);

	u32 framecounter = 0;

	//RASTER_enable(); // enable raster interrupts

	PCTRL_fade_in(FADE_DUR);

	while(1) {
		if(framecounter%64 == 0)
			SFX_play(SFX_mop);

		int angle = (framecounter * 8)&1023;
		//fix16 cam_cx = fix16Add(FIX16(160), fix16Mul(sinFix16(angle), FIX16(40)));
		//fix16 cam_cy = fix16Add(FIX16(96), fix16Mul(cosFix16(angle), FIX16(40)));
		//PLF_cam_to(cam_cx, cam_cy); spins camera around
		fix16 dweep_z = fix16Mul(sinFix16(angle), FIX16(10));
		dweep_z = dweep_z<0?fix16Neg(dweep_z):dweep_z;
		player_pf_z = dweep_z;

		bool clicked = MOUSE_step();
		if(clicked)
		{
			s16 click_fp_x = mouse_x/16;
			s16 click_fp_y = mouse_y/16;

			if(click_fp_y < 12)
			{
				PlfTile tile = PLF_get_tile(click_fp_x, click_fp_y);
				if(tile.attrs & PLF_ATTR_SOLID)
				{
					SFX_play(SFX_burn);
				}
				else
				{
					SFX_play(SFX_go4);
					dest_pf_x = FIX16(click_fp_x);
					dest_pf_y = FIX16(click_fp_y);
				}
			}
			else
			{
				// hud click
				SFX_play(SFX_no);
			}
		}

		if(player_pf_x != dest_pf_x)
		{
			fix16 delta = ((player_pf_x < dest_pf_x)?PLAYER_SPEED:-PLAYER_SPEED);
			player_pf_x += delta;
			if(player_pf_x == FIX16(17) && player_pf_y == FIX16(10))
			{
				XGM_startPlay(bgm_victory);
				XGM_setLoopNumber(0);
			}

			SPR_setHFlip(spr_player, delta<0);
		}

		if(player_pf_y != dest_pf_y)
		{
			player_pf_y += ((player_pf_y < dest_pf_y)?PLAYER_SPEED:-PLAYER_SPEED);
			if(player_pf_x == FIX16(17) && player_pf_y == FIX16(10))
			{
				XGM_startPlay(bgm_victory);
				XGM_setLoopNumber(0);
			}
		}

		positionPlayer();
		positionShadow();
		scaleShadow();

		if(framecounter % 16 == 0)
		{
			// TODO add this functionality to PCTRL in generic fashion
			u16 tmp = PCTRL_src_lines[PAL_LINE_BG_0*16+12];
			PCTRL_src_lines[PAL_LINE_BG_0*16+12] = PCTRL_src_lines[PAL_LINE_BG_0*16+13];
			PCTRL_src_lines[PAL_LINE_BG_0*16+13] = PCTRL_src_lines[PAL_LINE_BG_0*16+14];
			PCTRL_src_lines[PAL_LINE_BG_0*16+14] = tmp;
		}

		if(framecounter % 8 == 0)
		{
			// TODO add this functionality to PCTRL in generic fashion
			u16 tmp = PCTRL_src_lines[PAL_LINE_BG_1*16+15];
			PCTRL_src_lines[PAL_LINE_BG_1*16+15] = PCTRL_src_lines[PAL_LINE_BG_1*16+14];
			PCTRL_src_lines[PAL_LINE_BG_1*16+14] = tmp;
		}

		if(FALSE)//framecounter % 6 == 0)
		{
			// TODO add this functionality to PCTRL in generic fashion
			u16 tmp = PCTRL_src_lines[PAL_LINE_BG_0*16+17];
			PCTRL_src_lines[PAL_LINE_BG_0*16+17] = PCTRL_src_lines[PAL_LINE_BG_0*16+18];
			PCTRL_src_lines[PAL_LINE_BG_0*16+18] = PCTRL_src_lines[PAL_LINE_BG_0*16+19];
			PCTRL_src_lines[PAL_LINE_BG_0*16+19] = PCTRL_src_lines[PAL_LINE_BG_0*16+20];
			PCTRL_src_lines[PAL_LINE_BG_0*16+20] = PCTRL_src_lines[PAL_LINE_BG_0*16+21];
			PCTRL_src_lines[PAL_LINE_BG_0*16+21] = tmp;
		}

		if(FALSE)//framecounter % 4 == 0)
		{
			// TODO add this functionality to PCTRL in generic fashion
			u16 tmp = PCTRL_src_lines[PAL_LINE_BG_0*16+8];
			PCTRL_src_lines[PAL_LINE_BG_0*16+8] = PCTRL_src_lines[PAL_LINE_BG_0*16+9];
			PCTRL_src_lines[PAL_LINE_BG_0*16+9] = PCTRL_src_lines[PAL_LINE_BG_0*16+10];
			PCTRL_src_lines[PAL_LINE_BG_0*16+10] = PCTRL_src_lines[PAL_LINE_BG_0*16+11];
			PCTRL_src_lines[PAL_LINE_BG_0*16+11] = tmp;
		}

		PLF_update_scroll(FALSE); // update playfield scroll (no force redraw)
		SPR_update(); // step sprite system
		PCTRL_step(); // evaluate palettes for next frame
		SYS_doVBlankProcess();
		framecounter++;
	}

// END IMAGE TEST

	return 0;
}
