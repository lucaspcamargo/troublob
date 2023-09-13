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

    INPUT_center_cursor();
    INPUT_set_cursor_visible(TRUE);

    // init playfield and hud
    PLF_init(level_id);
    HUD_init();
    PLR_init();

    { // setup hud inventory
        enum ToolId all_tools[10];
        all_tools[0] = TOOL_MOVE;
        memcpy(all_tools + 1, curr_lvl->tool_inventory, 9);
        HUD_inventory_set(all_tools);
    }

    XGM_startPlay(curr_lvl->bgm_xgm);
    PCTRL_fade_in(PAL_STD_FADE_DURATION);


    u32 framecounter = 0;
    Sprite * item_preview = NULL;
    const SpriteDefinition * item_preview_def = NULL;

    for(;;)
    {

        // process input and get info
        INPUT_step();
        bool im_change;
        u16 changed, state;
        INPUT_get_last_state(&im_change, &changed, &state);
        enum InputMethod meth;
        meth = INPUT_get_curr_method();
        s16 mouse_x, mouse_y;
        INPUT_get_cursor_position(&mouse_x, &mouse_y);
        bool l_click = meth == INPUT_METHOD_MOUSE && BUTTON_LMB & changed & state;
        bool r_click = meth == INPUT_METHOD_MOUSE && BUTTON_RMB & changed & state;

        enum ToolId curr_tool = HUD_inventory_curr();
        bool mouse_in_field = mouse_y < 192;

        if(r_click && curr_tool != TOOL_MOVE)
        {
            SFX_play(SFX_dull);
            HUD_inventory_set_curr_idx(0);
            curr_tool = HUD_inventory_curr();
        }

        if(mouse_in_field)
        {
            s16 mouse_pf_x = mouse_x / 16;
            s16 mouse_pf_y = mouse_y / 16;

            ToolQuery tq;
            TOOL_query(curr_tool, mouse_pf_x, mouse_pf_y, &tq);
            INPUT_set_cursor(tq.cursor);

            // handle item preview existance
            if(item_preview_def != tq.preview_sprite)
            {
                if(item_preview)
                {
                    SPR_releaseSprite(item_preview);
                    item_preview = NULL;
                }
                item_preview_def = tq.preview_sprite;
                if(item_preview_def)
                {
                    item_preview = SPR_addSprite(item_preview_def,
                                                 16*mouse_pf_x,
                                                 16*mouse_pf_y-(item_preview_def->h-16),
                                                 TILE_ATTR(tq.preview_pal_line, 0, 0, 0));
                    SPR_setHFlip(item_preview, tq.preview_flip_h);
                    SPR_setAnim(item_preview, tq.preview_anim);
                }
            }

            if(item_preview)
            {
                const bool visible = framecounter%2 && tq.can_use;
                SPR_setVisibility(item_preview, visible? VISIBLE : HIDDEN);
                if(visible)
                {
                    SPR_setPosition(item_preview, 16*mouse_pf_x, 16*mouse_pf_y-(item_preview_def->h-16));
                    SPR_setDepth(item_preview, PLF_get_sprite_depth(intToFix16(mouse_pf_x), intToFix16(mouse_pf_y)));
                    if(tq.preview_sprite) // tool query has preview sprite info now
                    {
                        if(tq.preview_anim != item_preview->animInd)
                            SPR_setAnim(item_preview, tq.preview_anim);
                        SPR_setHFlip(item_preview, tq.preview_flip_h);
                    }
                }
            }

            if(tq.can_use && l_click) // clicked
            {
                // field click
                TOOL_exec(curr_tool, mouse_pf_x, mouse_pf_y);
                if(curr_tool != TOOL_MOVE)
                    HUD_inventory_pop_curr();
            }
        }
        else
        {
            // mouse on hud
            INPUT_set_cursor(INPUT_CURSOR_NORMAL);

            if(item_preview)
                SPR_setVisibility(item_preview, HIDDEN);

            if(l_click)
            {
                HUD_on_click(mouse_x, mouse_y);
                curr_tool = HUD_inventory_curr();
            }
        }


        PLR_update(framecounter);
        PLF_update_objects(framecounter);
        HUD_update();


        // Done with game logic, now update render systems

        PCTRL_step(framecounter); // evaluate palettes for next frame

        if (DEBUG_CPU_LOAD)
            SYS_showFrameLoad(FALSE);

        PLF_update_scroll(FALSE); // update playfield scroll (no force redraw)

        SPR_update(); // step sprite system
        SYS_doVBlankProcess();
        framecounter++;

    }  // end main for

    // TODO deinit stuff
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
    next_cmd.cmd = DEBUG_MENU? DIREC_CMD_DEBUG_MENU : DIREC_CMD_TITLE;

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
