#include "dweep_config.h"

#include <genesis.h>
#include "resources.h"
#include "string.h"

#include "palette_ctrl.h"
#include "title.h"
#include "playfield.h"
#include "hud.h"
#include "raster.h"
#include "input.h"
#include "sfx.h"


enum PlayerState {
    PLR_STATE_IDLE = 0,
    PLR_STATE_MOVING_PATH
};

int main(bool hard) {

    if(!hard)
        SYS_hardReset();

    // init vdp
    VDP_setScreenWidth320();
    SYS_doVBlankProcess();

    // init subsystems
    SPR_init();
    PCTRL_init();
    INPUT_init();
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
    fix16 player_pf_x;
    fix16 player_pf_y;
    PLF_player_get_initial_pos(&player_pf_x, &player_pf_y);
    fix16 player_pf_z = FIX16(0);
    fix16 dest_pf_x = player_pf_x;
    fix16 dest_pf_y = player_pf_y;
    fix16 final_dest_pf_x = player_pf_x;
    fix16 final_dest_pf_y = player_pf_y;
    bool player_float = FALSE;
    enum PlayerState player_state = PLR_STATE_IDLE;

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

    //Play a song
    const void * const songs[] = {bgm_stage_1, bgm_stage_2, bgm_stage_3, bgm_stage_4, bgm_stage_5};
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

        bool clicked = INPUT_step();
        if(clicked)
        {
            s16 click_pf_x = mouse_x/16;
            s16 click_pf_y = mouse_y/16;

            if(click_pf_y < 12)
            {
                // field click
                PlfTile tile = PLF_get_tile(click_pf_x, click_pf_y);
                if(tile.attrs & PLF_ATTR_SOLID)
                {
                    SFX_play(SFX_no);
                }
                else
                {
                    bool found = PLF_player_pathfind(fix16ToInt(player_pf_x),fix16ToInt(player_pf_y), click_pf_x, click_pf_y);
                    if (found)
                    {
                        SFX_play(SFX_go1+(random()%3));
                        player_state = PLR_STATE_MOVING_PATH;
                        final_dest_pf_x = FIX16(click_pf_x);
                        final_dest_pf_y = FIX16(click_pf_y);
                        u16 bufx, bufy;
                        bool found_next_pos = PLF_player_path_next(fix16ToInt(player_pf_x), fix16ToInt(player_pf_y), &bufx, &bufy);
                        if(found_next_pos)
                        {
                            dest_pf_x = FIX16(bufx);
                            dest_pf_y = FIX16(bufy);
                        }
                    }
                    else
                        SFX_play(SFX_water);
                }
            }
            else
            {
                // hud click
                SFX_play(SFX_no);
            }
        }

        if(player_state == PLR_STATE_MOVING_PATH)
        {
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
                //PlfTile tile = PLF_get_tile(fix16ToInt(player_pf_x), fix16ToInt(player_pf_y));
                //player_float = (tile.attrs & PLF_ATTR_HOT)? TRUE : FALSE;
                if(player_pf_x == dest_pf_x && player_pf_y == dest_pf_y)
                {
                    // arrived at immediate dest, either we arrived at final destination, or we find our next imm. dest.
                    if(player_pf_x == final_dest_pf_x && player_pf_y == final_dest_pf_y)
                    {
                        player_state = PLR_STATE_IDLE;
                    }
                    else
                    {
                        u16 bufx, bufy;
                        bool found_next_pos = PLF_player_path_next(fix16ToInt(player_pf_x), fix16ToInt(player_pf_y), &bufx, &bufy);
                        if(found_next_pos)
                        {
                            dest_pf_x = FIX16(bufx);
                            dest_pf_y = FIX16(bufy);
                        }
                        else player_state = PLR_STATE_IDLE;
                    }
                }
            }
        }

        positionPlayer();
        positionShadow();
        scaleShadow();

        if(DEBUG_PLAYER)
        {
            char buf[20];
            sprintf(buf, "S%X@%X,%X,%X",
                    (int) player_state,
                    (int) player_pf_x, (int) player_pf_y, (int) player_pf_z);
            VDP_drawText(buf, 0, 24);
            sprintf(buf, "%X,%X", (int) dest_pf_x, (int) dest_pf_y);
            VDP_drawText(buf, 20, 24);
            sprintf(buf, "%X,%X", (int) final_dest_pf_x, (int) final_dest_pf_y);
            VDP_drawText(buf, 30, 24);
        }

        PCTRL_step(framecounter); // evaluate palettes for next frame

        if (DEBUG_CPU_LOAD)
            SYS_showFrameLoad(FALSE);

        // BUG: calling the below every frame crashes the game!!!!
        //      if I ever make scrolling levels, will need to figure out why
        //PLF_update_scroll(FALSE); // update playfield scroll (no force redraw)

        SPR_update(); // step sprite system
        SYS_doVBlankProcess();
        framecounter++;
    }

// END IMAGE TEST

    return 0;
}
