#include "dweep_config.h"
#include "debug_menu.h"
#include "registry.h"
#include "sfx.h"
#include "input.h"
#include "resources.h"


#define POS_X_TXT 4
#define POS_Y_TXT 2
#define POS_X_OPT (POS_X_TXT + 7)
#define POS_X_EXTRA (POS_X_TXT + 13)
#define POS_X_ARROW (POS_X_TXT - 2)

#define NUM_ENTRIES 4
#define NUM_SUBOPTS 3
static int curr_idx = 0;
static int curr_subopts[NUM_SUBOPTS];
static int subopt_min[NUM_SUBOPTS];
static int subopt_max[NUM_SUBOPTS];

static const u8 * bgms[] = {
    bgm_stage_1,
    bgm_stage_2,
    bgm_stage_3,
    bgm_stage_4,
    bgm_stage_5,
    bgm_victory,
    bgm_defeat,
    bgm_stage_1};

// quick and dirty
#define VDP_drawText(text, x, y) VDP_drawTextEx(BG_A, text, TILE_ATTR(PAL_LINE_HUD, 0, 0, 0), x, y, DMA)

void debug_menu_draw_cursor()
{
    const char arrow[2] = {'z'+5, '\0'};
    for(int i = 0; i < NUM_ENTRIES; i++)
        VDP_drawText(curr_idx == i? arrow : " ", POS_X_ARROW, POS_Y_TXT+i);
}

void debug_menu_draw_opts()
{
    char option[5];
    for(int i = 0; i < NUM_SUBOPTS; i++)
    {
        intToHex(curr_subopts[i], option, 4);
        VDP_drawText(option, POS_X_OPT, POS_Y_TXT+i);
    }

    // special
    VDP_fillTileMapRect(BG_A, TILE_FONT_INDEX, POS_X_EXTRA, POS_Y_TXT, (320/8)-POS_X_EXTRA, 1);
    if(curr_subopts[0] < RGST_lvl_count)
        VDP_drawText(RGST_levels[curr_subopts[0]].name, POS_X_EXTRA, POS_Y_TXT);
}

bool debug_menu_option_inc(u8 opt_idx, bool dec)
{
    u16 delta = dec? -1 : 1;
    u16 new = curr_subopts[opt_idx] + delta;
    new = ((s16)new) < ((s16)subopt_min[opt_idx])? subopt_max[opt_idx] : new;
    new = ((s16)new) > ((s16)subopt_max[opt_idx])? subopt_min[opt_idx] : new;
    if( new != curr_subopts[opt_idx] )
    {
        curr_subopts[opt_idx] = new;
        SFX_play(opt_idx == 2? new : SFX_ding);
        return TRUE;
    }
    return FALSE;
}

enum DirectorCommandType debug_menu_option_exec(u8 opt_idx, enum DirectorCommandFlags *outflags)
{
    switch(opt_idx)
    {
        case 0:
            (*outflags) = DIREC_CMD_F_NONE;
            return DIREC_CMD_LEVEL;
        case 1:
            XGM_stopPlay();
            XGM_startPlay(bgms[curr_subopts[1]]);
            break;
        case 2:
            SFX_play(curr_subopts[2]);
            break;
        case 3:
            (*outflags) = DIREC_CMD_F_NONE;
            return DIREC_CMD_TITLE;
    }
    return DIREC_CMD_INVAL;
}

void exec_debug_menu(DirectorCommand *next_cmd)
{
    memset(next_cmd, 0, sizeof(DirectorCommand));

    // init params
    memset(subopt_min, 0x00, sizeof(subopt_min));
    subopt_max[0] = RGST_lvl_count - 1;
    subopt_max[1] = sizeof(bgms)/sizeof(u8*) - 1;
    subopt_min[2] = SFX_USER_OFFSET;
    subopt_max[2] = SFX_END - 1;
    memcpy(curr_subopts, subopt_min, sizeof(curr_subopts));
    curr_subopts[0] = 1;

    PCTRL_set_line(0, pal_tset_hud.data);
    u16 dangerPal[16];
    memcpy(dangerPal, pal_tset_hud.data, 32);
    for(int i = 0; i < 16; i++)
        if(i != 6)
            dangerPal[i] = dangerPal[i] & RGB3_3_3_TO_VDPCOLOR(0b111, 0, 0);
    PCTRL_set_line(1, dangerPal);

    PCTRL_set_line(PAL_LINE_HUD, pal_tset_hud.data);
    PCTRL_fade_in(0);
    PCTRL_step(0);
    SYS_doVBlankProcess();

    VDP_fillTileMapRect(BG_A, TILE_FONT_INDEX, 0, 0, 320/8, 224/8-(DEBUG_INPUT?1:0));
    VDP_drawText("LEVEL <    >", POS_X_TXT, POS_Y_TXT);
    VDP_drawText("BGM   <    >", POS_X_TXT, POS_Y_TXT + 1);
    VDP_drawText("SFX   <    >", POS_X_TXT, POS_Y_TXT + 2);
    VDP_drawText("TITLE", POS_X_TXT, POS_Y_TXT + 3);

    if(!INPUT_is_mouse_present())
        VDP_drawTextEx(BG_A, "USING A MOUSE IS RECOMMENDED", TILE_ATTR(1,0,0,0), 6, 18, DMA);

    VDP_drawText("PORT 1", 12, 20);
    VDP_drawText("PORT 2", 21, 20);
    Sprite *port1 = SPR_addSprite(&spr_input_dev, 160-64-4, 168, 0);
    SPR_setAnimAndFrame(port1, 0, INPUT_get_input_dev_icon(PORT_1));
    Sprite *port2 = SPR_addSprite(&spr_input_dev, 160+4, 168, 0);
    SPR_setAnimAndFrame(port2, 0, INPUT_get_input_dev_icon(PORT_2));

    debug_menu_draw_cursor();
    debug_menu_draw_opts();

    INPUT_set_cursor_visible(INPUT_is_mouse_present());

    for(;;)
    {
        bool redraw_cursor = FALSE;
        bool redraw_opts = FALSE;

        INPUT_step();
        bool im_change;
        u16 changed, state;
        INPUT_get_last_state(&im_change, &changed, &state);
        enum InputMethod meth;
        meth = INPUT_get_curr_method();
        if(im_change)
            INPUT_set_cursor_visible(meth == INPUT_METHOD_MOUSE);
        if(meth == INPUT_METHOD_PAD && changed)
        {
            if( BUTTON_UP & changed & state )
                if(curr_idx)
                {
                    curr_idx --;
                    redraw_cursor = TRUE;
                    SFX_play(SFX_dull);
                }
            if( BUTTON_DOWN & changed & state )
                if(curr_idx < NUM_ENTRIES - 1)
                {
                    curr_idx ++;
                    redraw_cursor = TRUE;
                    SFX_play(SFX_dull);
                }

            if( curr_idx < NUM_SUBOPTS && (BUTTON_LEFT|BUTTON_RIGHT) & changed & state )
            {

                if(debug_menu_option_inc(curr_idx, BUTTON_LEFT & changed & state))
                    redraw_opts = TRUE;
            }

            if( BUTTON_A & changed & state )
            {
                enum DirectorCommandType type;
                enum DirectorCommandFlags flags;
                if((type = debug_menu_option_exec(curr_idx, &flags)) != DIREC_CMD_INVAL)
                {
                    next_cmd->type = type;
                    next_cmd->flags = flags;
                    break;
                }
            }

            if( BUTTON_START & changed & state )
                break;
        }
        else if(meth == INPUT_METHOD_MOUSE && changed)
        {
            bool l_click = meth == INPUT_METHOD_MOUSE && BUTTON_LMB & changed & state;
            if(l_click)
            {
                s16 tx, ty;
                INPUT_get_cursor_position(&tx, &ty);
                tx /= 8;
                ty /= 8;

                if(tx >= POS_X_TXT && tx < (320/8)-POS_X_TXT && ty >= POS_Y_TXT && ty < (POS_Y_TXT+NUM_ENTRIES) )
                {
                    tx -= POS_X_TXT;
                    ty -= POS_Y_TXT;

                    if(tx >=5 && tx <= 7 && ty < NUM_SUBOPTS)
                    {
                        if(debug_menu_option_inc(ty, TRUE))
                            redraw_opts = TRUE;
                    }
                    else if(tx >=10 && tx <= 12 && ty < NUM_SUBOPTS)
                    {
                        if(debug_menu_option_inc(ty, FALSE))
                            redraw_opts = TRUE;
                    }
                    else
                    {
                        enum DirectorCommandType type;
                        enum DirectorCommandFlags flags;
                        if((type = debug_menu_option_exec(ty, &flags)) != DIREC_CMD_INVAL)
                        {
                            next_cmd->type = type;
                            next_cmd->flags = flags;
                            break;
                        }
                    }
                }
                else
                {
                    // clicked outside the menu
                    next_cmd->type = DIREC_CMD_LEVEL;
                };
            }
        }

        if(redraw_cursor)
            debug_menu_draw_cursor();

        if(redraw_opts)
            debug_menu_draw_opts();

        SPR_update();
        SYS_doVBlankProcess();
    }

    XGM_stopPlay();
    INPUT_set_cursor_visible(FALSE);
    SPR_setVisibility(port1, HIDDEN);
    SPR_setVisibility(port2, HIDDEN);
    SPR_update();
    SYS_doVBlankProcess();
    SPR_releaseSprite(port1);
    SPR_releaseSprite(port2);
    SPR_update();
    SYS_doVBlankProcess();

    if(next_cmd->type == DIREC_CMD_LEVEL)
    {
        next_cmd->arg0 = curr_subopts[0];
        next_cmd->arg1 = 0;
        next_cmd->arg_p = NULL;
    }
}
