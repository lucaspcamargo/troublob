#include "dweep_config.h"

#include <genesis.h>
#include "resources.h"
#include "string.h"

#include "palette_ctrl.h"
#include "title2.h"
#include "playfield.h"
#include "hud.h"
#include "player.h"
#include "raster.h"
#include "input.h"
#include "sfx.h"



int exec_playfield(u16 level_id){
    // BEGIN main section
    // init playfield and hud
    PLF_init(0);
    HUD_init();
    PLR_init();


    //Play a song
    const void * const songs[] = {bgm_stage_1, bgm_stage_2, bgm_stage_3, bgm_stage_4, bgm_stage_5};
    XGM_startPlay(songs[1]);

    u32 framecounter = 0;

    //RASTER_enable(); // enable raster interrupts

    PCTRL_fade_in(FADE_DUR);

    for(;;) {

        bool clicked = INPUT_step();
        if(clicked)
        {
            s16 click_pf_x, click_pf_y;
            INPUT_get_cursor_position(&click_pf_x, &click_pf_y);
            click_pf_x /= 16;
            click_pf_y /= 16;

            if(click_pf_y < 12)
            {
                // field click
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
                SFX_play(SFX_no);
            }
        }

        PLR_update(framecounter);

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

    exec_playfield(0);

    return 0;
}
