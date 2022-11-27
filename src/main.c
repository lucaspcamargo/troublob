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


int main(bool hard) {

    if(!hard)
        SYS_hardReset();

    // init vdp
    VDP_setScreenWidth320();
    SYS_doVBlankProcess();

    // init subsystems
    SPR_init();
    PCTRL_init();
    MOUSE_init();
    SFX_register_all();
    SYS_doVBlankProcess();

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
    bool player_float = FALSE;

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

    for(;;) {
        int angle = (framecounter * 4)&1023;
        if(framecounter%64 == 0)
            SFX_play(SFX_mop);

        if(player_float)
        {
            player_pf_z = FIX16(6)+fix16Mul(sinFix16(angle/2), FIX16(4));
        }
        else
        {
            player_pf_z = fix16Mul(sinFix16(angle*2), FIX16(10));
            player_pf_z = player_pf_z<0?fix16Neg(player_pf_z):player_pf_z;
        }

        bool clicked = MOUSE_step();
        if(clicked)
        {
            s16 click_pf_x = mouse_x/16;
            s16 click_pf_y = mouse_y/16;

            if(click_pf_y < 12)
            {
                PlfTile tile = PLF_get_tile(click_pf_x, click_pf_y);
                if(0)//tile.attrs & PLF_ATTR_SOLID)
                {
                    SFX_play(SFX_burn);
                }
                else
                {
                    SFX_play(SFX_go4);
                    dest_pf_x = FIX16(click_pf_x);
                    dest_pf_y = FIX16(click_pf_y);
                    PLF_player_pathfind(fix16ToInt(player_pf_x),fix16ToInt(player_pf_y),fix16ToInt(dest_pf_x),fix16ToInt(dest_pf_y));
                }
            }
            else
            {
                // hud click
                SFX_play(SFX_no);
            }
        }

        bool changed = FALSE;

        if(player_pf_x != dest_pf_x)
        {
            fix16 delta = ((player_pf_x < dest_pf_x)?PLAYER_SPEED:-PLAYER_SPEED);
            player_pf_x += delta;
            changed = TRUE;
            SPR_setHFlip(spr_player, delta<0);
        }

        if(player_pf_y != dest_pf_y)
        {
            player_pf_y += ((player_pf_y < dest_pf_y)?PLAYER_SPEED:-PLAYER_SPEED);
            changed = TRUE;
        }

        if(changed && fix16Frac(player_pf_x)==0 && fix16Frac(player_pf_y)==0)
        {
            // arrived on a tile
            if(player_pf_x == FIX16(17) && player_pf_y == FIX16(10))
            {
                //XGM_setLoopNumber(0);
            }
            else
            {
                // get arrived tile attrs
                PlfTile tile = PLF_get_tile(fix16ToInt(player_pf_x), fix16ToInt(player_pf_y));
                player_float = (tile.attrs & PLF_ATTR_HOT)? TRUE : FALSE;
            }
        }

        positionPlayer();
        positionShadow();
        scaleShadow();

        PLF_update_scroll(FALSE); // update playfield scroll (no force redraw)
        SPR_update(); // step sprite system
        PCTRL_step(framecounter); // evaluate palettes for next frame

        if (DEBUG_CPU_LOAD)
            SYS_showFrameLoad(FALSE);

        SYS_doVBlankProcess();
        framecounter++;
    }

// END IMAGE TEST

    return 0;
}
