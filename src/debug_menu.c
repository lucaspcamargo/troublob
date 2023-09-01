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

#define NUM_ENTRIES 3
#define NUM_SUBOPTS 3
static int curr_idx = 0;
static int curr_subopts[NUM_SUBOPTS];


// quick and dirty
#define VDP_drawText(text, x, y) VDP_drawTextEx(BG_A, text, TILE_ATTR(PAL_LINE_HUD, 0, 0, 0), x, y, DMA)

void debug_menu_draw_cursor()
{
    const char arrow[2] = {'z'+5, '\0'};
    for(int i = 0; i < NUM_SUBOPTS; i++)
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
}

void exec_debug_menu(DirectorCommand *next_cmd)
{
    PCTRL_set_source(0, pal_tset_hud.data);
    PCTRL_set_source(PAL_LINE_HUD, pal_tset_hud.data);
    PCTRL_fade_in(0);
    PCTRL_step(0);
    SYS_doVBlankProcess();

    VDP_fillTileMapRect(BG_A, TILE_FONT_INDEX, 0, 0, 320/8, 224/8);
    VDP_drawText("LEVEL <    >", POS_X_TXT, POS_Y_TXT);
    VDP_drawText("BGM   <    >", POS_X_TXT, POS_Y_TXT + 1);
    VDP_drawText("SFX   <    >", POS_X_TXT, POS_Y_TXT + 2);

    VDP_drawText(RGST_levels[curr_subopts[0]].name, POS_X_EXTRA, POS_Y_TXT);

    debug_menu_draw_cursor();
    debug_menu_draw_opts();

    INPUT_set_cursor_visible(TRUE);

    for(int i = 0; i < NUM_SUBOPTS; i++)
        curr_subopts[i] = 0;

    for(;;)
    {
        if(INPUT_step())
            break;
        SPR_update(); // for mouse cursor
        SYS_doVBlankProcess();
    }

    next_cmd->cmd = DIREC_CMD_LEVEL;
    next_cmd->flags = DIREC_CMD_F_NONE;
    next_cmd->arg0 = 0;
    next_cmd->arg1 = 0;
    next_cmd->arg_p = NULL;
}
