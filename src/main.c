#include "dweep_config.h"
#include "dweep_globals.h"

#include <genesis.h>
#include "resources.h"
#include "string.h"

#include "palette_ctrl.h"
#include "director.h"
#include "debug_menu.h"
#include "title2.h"
#include "playfield.h"
#include "hud.h"
#include "player.h"
#include "raster.h"
#include "input.h"
#include "registry.h"
#include "sfx.h"


int exec_playfield(const DirectorCommand *curr_cmd, DirectorCommand *next_cmd){

    (void) next_cmd; // future usage
    const u16 level_id = curr_cmd->arg0;
    const RGST_lvl * curr_lvl = RGST_levels + level_id;

    INPUT_set_cursor_visible(TRUE);

    // init playfield and hud
    PLF_init(level_id);
    HUD_init();
    PLR_init();

    XGM_startPlay(curr_lvl->bgm_xgm);

    u32 framecounter = 0;

    PCTRL_fade_in(PAL_STD_FADE_DURATION);

    for(;;) {

        bool clicked = INPUT_step();
        if(clicked)
        {
            s16 click_x, click_y;
            INPUT_get_cursor_position(&click_x, &click_y);

            if(click_y < 192)
            {
                // field click
                s16 click_pf_x = click_x / 16;
                s16 click_pf_y = click_y / 16;

                PlfTile tile = *PLF_get_tile(click_pf_x, click_pf_y);
                if(tile.attrs & PLF_ATTR_SOLID)
                {
                    SFX_play(SFX_no);
                }
                else
                {
                    PLR_goto(click_pf_x, click_pf_y);
                }
            }
            else
            {
                // hud click
                static bool dialog_test = TRUE;
                if(dialog_test)
                    HUD_dialog_start(0, 1);
                else
                    HUD_dialog_end();
                dialog_test = !dialog_test;
            }
        }

        PLR_update(framecounter);
        HUD_update();


        // Done with game logic, now update render systems

        PCTRL_step(framecounter); // evaluate palettes for next frame

        if (DEBUG_CPU_LOAD)
            SYS_showFrameLoad(FALSE);

        PLF_update_scroll(FALSE); // update playfield scroll (no force redraw)

        SPR_update(); // step sprite system
        SYS_doVBlankProcess();
        framecounter++;
    }

    // end main for
}

int main(bool hard) {

    if(!hard)
        SYS_hardReset();

    GLOBAL_vdp_tile_watermark = TILE_USER_INDEX;

    // init vdp
    VDP_setScreenWidth320();
    SYS_doVBlankProcess();

    // init subsystems
    SPR_init();
    PCTRL_init();
    INPUT_init();
    SFX_register_all();
    HUD_preinit(); // init global/UI graphics
    SYS_doVBlankProcess();

    DirectorCommand next_cmd;
    DirectorCommand curr_cmd;

    memset(&next_cmd, 0x00, sizeof(DirectorCommand));
    next_cmd.cmd = DEBUG_MENU&&0? DIREC_CMD_DEBUG_MENU : DIREC_CMD_TITLE;

    for(;;)
    {
        curr_cmd = next_cmd;
        switch(curr_cmd.cmd)
        {
            case DIREC_CMD_DEBUG_MENU:
                exec_debug_menu(&next_cmd);
                break;
            case DIREC_CMD_TITLE:
                TITLE_main(&curr_cmd, &next_cmd);
                break;
            case DIREC_CMD_LEVEL:
                // ensure we are blacked out
                PCTRL_fade_out(0);
                PCTRL_step(0);
                SYS_doVBlankProcess();

                exec_playfield(&curr_cmd, &next_cmd);
                break;
        }
    }





    return 0;
}


/* HINT EXAMPLE

HINTERRUPT_CALLBACK hint()
{
  PAL_setColor(0, 0x0E0E);
}

TO USE:
    VDP_setHIntCounter(192);
    SYS_setHIntCallback(hint);
    VDP_setHInterrupt(TRUE);
*/
