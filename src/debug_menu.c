#include "dweep_config.h"
#include "debug_menu.h"
#include "registry.h"
#include "sfx.h"
#include <genesis.h>

#define POS_X_TXT 4
#define POS_Y_TXT 2
#define POS_X_OPT (POS_X_TXT + 7)
#define POS_X_ARROW (POS_X_TXT - 2)

#define NUM_OPTS 3
static int curr_idx = 0;
static int curr_subopts[NUM_OPTS];

void debug_menu_draw_cursor()
{
    const char arrow[2] = {'z'+5, '\0'};
    for(int i = 0; i < NUM_OPTS; i++)
        VDP_drawText(curr_idx == i? arrow : " ", POS_X_ARROW, POS_Y_TXT+i);
}

void debug_menu_draw_opts()
{
    char option[5];
    for(int i = 0; i < NUM_OPTS; i++)
    {
        intToHex(curr_subopts[i], option, 4);
        VDP_drawText(option, POS_X_OPT, POS_Y_TXT+i);
    }
}

int exec_debug_menu(DirectorCommand *next_cmd)
{

    VDP_drawText("LEVEL <    >", POS_X_TXT, POS_Y_TXT);
    VDP_drawText("BGM   <    >", POS_X_TXT, POS_Y_TXT + 1);
    VDP_drawText("SFX   <    >", POS_X_TXT, POS_Y_TXT + 2);

    debug_menu_draw_cursor();
    debug_menu_draw_opts();

    for(int i = 0; i < NUM_OPTS; i++)
        curr_subopts[i] = 0;

    for(;;)
    {
        SYS_doVBlankProcess();
    }

    next_cmd->cmd = DIREC_CMD_LEVEL;
    next_cmd->flags = DIREC_CMD_F_NONE;
    next_cmd->arg0 = 0;
    next_cmd->arg1 = 0;
    next_cmd->arg_p = NULL;
}
