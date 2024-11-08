/*
This file is part of Blob Genesis.

Blob Genesis is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

Blob Genesis is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with Foobar. If not, see <https://www.gnu.org/licenses/>.
*/

#include "game_config.h"
#include "game_globals.h"

#include <genesis.h>
#include "resources.h"
#include "string.h"

#include "bgm.h"
#include "director.h"
#include "debug_menu.h"
#include "gfx_utils.h"
#include "hud.h"
#include "input.h"
#include "palette_ctrl.h"
#include "player.h"
#include "playfield.h"
#include "raster.h"
#include "registry.h"
#include "sfx.h"
#include "save.h"
#include "title2.h"


Sprite * epf_item_preview = NULL;
const SpriteDefinition * epf_item_preview_def = NULL;
bool epf_paused;
bool epf_redraw_plane_a;


void exec_playfield_setup(u16 level_id,const RGST_lvl * curr_lvl, bool initial);
void exec_playfield_input(u32 framecounter, bool *reset_flag);
void exec_playfield_pause_toggled();


int exec_playfield(const DirectorCommand *curr_cmd, DirectorCommand *next_cmd){

    (void) next_cmd; // future usage
    const u16 level_id = curr_cmd->arg0;
    const RGST_lvl * curr_lvl = RGST_levels + level_id;

    GFX_init_queue();

    exec_playfield_setup(level_id, curr_lvl, TRUE);

    u32 framecounter = 0;
    epf_paused = FALSE;
    epf_redraw_plane_a = FALSE;

    u16 tmr_player_dead = 0;
    bool reset_flag = FALSE;

    for(;;)
    {
        exec_playfield_input(framecounter, &reset_flag);

        if(!epf_paused)
        {
            PLR_update(framecounter);
            PLF_update_objects(framecounter);

            if(PLR_curr_state() == PLR_STATE_DEAD)
            {
                if(!tmr_player_dead)
                {
                    tmr_player_dead = 170;
                    BGM_stop_all();
                }
                else if(!--tmr_player_dead)
                    reset_flag = TRUE;
                else if(tmr_player_dead == 150)
                {
                    BGM_play_loops(bgm_defeat, 0);
                }
            }
        }

        if(reset_flag)
        {
            u32 subfc = 0;
            PCTRL_fade_out(PAL_STD_FADE_DURATION);
            while(subfc <= PAL_STD_FADE_DURATION)
            {
                PCTRL_step(framecounter + subfc); // evaluate palettes for next frame
                SYS_doVBlankProcess();
                subfc++;
            }
            exec_playfield_setup(level_id, curr_lvl, FALSE);
            reset_flag = FALSE;
        }


        // Done with game logic, now update render systems

        PCTRL_step(framecounter); // evaluate palettes for next frame
        if (DEBUG_CPU_LOAD)
            SYS_showFrameLoad(FALSE);
        PLF_update_scroll(FALSE); // update playfield scroll (no force redraw)
        SPR_update(); // step sprite system

        /*
         * Here SGDK waits for vblank and does the following:
         * - updates sprite data
         * - updates palette with DMA
         * - does all other queued DMA
         * - XGM updates
         * - input polling
         */
        SYS_doVBlankProcess();

        // other things to update during vblank must go hereafter (assuming we still have vblank time)
        HUD_update_with_gfx(); // plane update, do right after vblank
        if(epf_redraw_plane_a)
        {
            epf_redraw_plane_a = FALSE;
            PLF_plane_a_refresh();

        }

        if(!epf_paused)
            framecounter++;

    }  // end main for


    if(epf_item_preview)
    {
        SPR_releaseSprite(epf_item_preview);
        epf_item_preview = NULL;
    }

    GFX_stop_queue();

    PLF_destroy();
    PLR_destroy();
    // HUD_destroy(); TODO this
}

void exec_playfield_setup(u16 level_id, const RGST_lvl * curr_lvl, bool initial)
{
    BGM_stop_all();
    if(initial)
        INPUT_center_cursor();
    INPUT_set_cursor_visible(TRUE);
    if(initial)
    {
        PLR_init();
        PLF_init(level_id);
        HUD_init();
    }
    else{
        PLF_reset(level_id);
        PLR_reset();
        HUD_init(); // TODO maybe too heavy handed for reset
    }
    PLR_reset_position();
    { // setup hud inventory
        enum ToolId all_tools[10];
        all_tools[0] = TOOL_MOVE;
        memcpy(all_tools + 1, curr_lvl->tool_inventory, 9);
        HUD_inventory_set(all_tools);
    }

    INPUT_set_cursor_visible(TRUE);
    BGM_play(curr_lvl->bgm_xgm);
    PCTRL_fade_in(PAL_STD_FADE_DURATION);
}

void exec_playfield_input(u32 framecounter, bool *reset_flag)
{

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

    if(meth == INPUT_METHOD_PAD)
    {
        INPUT_set_cursor_visible(TRUE);


        if(BUTTON_START & changed & state)
        {
            HUD_menu_toggle();
            epf_paused = (HUD_state_curr() == HUD_ST_MENU);
            exec_playfield_pause_toggled();
        }

        // special joypad handling (mouse emulation and immediate movement)
        if(BUTTON_Z & changed & state)
            r_click = TRUE; // simulate immediate walk

        if(BUTTON_UP & state)
            INPUT_do_move_abs(mouse_x, mouse_y - 2, TRUE);
        if(BUTTON_DOWN & state)
            INPUT_do_move_abs(mouse_x, mouse_y + 2, TRUE);
        if(BUTTON_LEFT & state)
            INPUT_do_move_abs(mouse_x - 2, mouse_y, TRUE);
        if(BUTTON_RIGHT & state)
            INPUT_do_move_abs(mouse_x + 2, mouse_y, TRUE);

        if(BUTTON_A & changed & state)
            l_click = TRUE;

        if(BUTTON_B & changed & state)
            HUD_inventory_sel_prev();

        if(BUTTON_C & changed & state)
            HUD_inventory_sel_next();
    }

    if(r_click && curr_tool != TOOL_MOVE)
    {
        SFX_play(SFX_dull);
        HUD_inventory_set_curr_idx(0);
        curr_tool = HUD_inventory_curr();
    }

    if(mouse_in_field && !epf_paused)
    {
        s16 mouse_pf_x = mouse_x / 16;
        s16 mouse_pf_y = mouse_y / 16;

        ToolQuery tq;
        TOOL_query(curr_tool, mouse_pf_x, mouse_pf_y, &tq);
        INPUT_set_cursor(tq.cursor);

        // handle item preview existance
        if(epf_item_preview_def != tq.preview_sprite)
        {
            if(epf_item_preview)
            {
                SPR_releaseSprite(epf_item_preview);
                epf_item_preview = NULL;
            }
            epf_item_preview_def = tq.preview_sprite;
            if(epf_item_preview_def)
            {
                epf_item_preview = SPR_addSprite(epf_item_preview_def,
                                                16*mouse_pf_x,
                                                16*mouse_pf_y-(epf_item_preview_def->h-16),
                                                TILE_ATTR(tq.preview_pal_line, 0, 0, 0));
                SPR_setHFlip(epf_item_preview, tq.preview_flip_h);
                SPR_setAnim(epf_item_preview, tq.preview_anim);
            }
        }

        if(epf_item_preview)
        {
            const bool visible = framecounter%2 && tq.can_use;
            SPR_setVisibility(epf_item_preview, visible? VISIBLE : HIDDEN);
            if(visible)
            {
                SPR_setPosition(epf_item_preview, 16*mouse_pf_x, 16*mouse_pf_y-(epf_item_preview_def->h-16));
                SPR_setDepth(epf_item_preview, PLF_get_sprite_depth(intToFix16(mouse_pf_x), intToFix16(mouse_pf_y)));
                if(tq.preview_sprite) // tool query has preview sprite info now
                {
                    if(tq.preview_anim != epf_item_preview->animInd)
                        SPR_setAnim(epf_item_preview, tq.preview_anim);
                    SPR_setHFlip(epf_item_preview, tq.preview_flip_h);
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

        if(epf_item_preview)
            SPR_setVisibility(epf_item_preview, HIDDEN);

        if(l_click)
        {
            HUD_on_click(mouse_x, mouse_y);
            curr_tool = HUD_inventory_curr();
            enum HUDState new_st = HUD_state_curr();
            bool paused_now = new_st == HUD_ST_MENU;
            if(paused_now != epf_paused)
            {
                epf_paused = paused_now;
                exec_playfield_pause_toggled();
            }
            if(new_st == HUD_ST_RESET_REQUEST)
                (*reset_flag) = TRUE;

        }
    }
}

void exec_playfield_pause_toggled()
{
    if(epf_paused)
        BGM_pause();
    else
    {
        BGM_resume();
        epf_redraw_plane_a = TRUE;
    }

    SFX_play(SFX_ding);
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
    next_cmd.type = DEBUG_MENU? DIREC_CMD_DEBUG_MENU : DIREC_CMD_TITLE;

    for(;;)
    {
        curr_cmd = next_cmd;
        switch(curr_cmd.type)
        {
            case DIREC_CMD_INVAL:
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
