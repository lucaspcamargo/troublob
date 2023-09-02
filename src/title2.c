#include <genesis.h>
#include "title2.h"

#include "dweep_config.h"
#include "resources.h"
#include "palette_ctrl.h"
#include "sfx.h"
#include "input.h"


#define TITLE_FRAMES (300 + 2 * PAL_STD_FADE_DURATION)
#define TITLE_DISCLAIMER_FRAMES 200


void _TITLE_disclaimer()
{
    const u16 txt_y = 16;

    VDP_fillTileMapRect(BG_B, TILE_FONT_INDEX, 0, 0, 320/8, 224/8);
    VDP_drawTextBG(BG_B, "Dweep Genesis ", 10, txt_y);
    VDP_drawTextBG(BG_B, GAME_VERSION, 24, txt_y);
    VDP_drawTextBG(BG_B, "by Lucas Pires Camargo", 8, txt_y+2);
    VDP_drawTextBG(BG_B, "Original game by Steve Pavlina", 4, txt_y+6);
    VDP_drawTextBG(BG_B, "Original music by Michael Huang", 4, txt_y+7);

    PCTRL_set_source(PAL_LINE_BG_0, sgdk_logo.palette->data);
    VDP_drawBitmapEx(BG_A, &sgdk_logo, TILE_ATTR(PAL_LINE_BG_0, 0, 0, 0), 16, 4, FALSE);

    u16 framecounter = 0;
    for(int i = 0; i < TITLE_DISCLAIMER_FRAMES; i++)
    {
        PCTRL_step(framecounter);
        SYS_doVBlankProcess();
        framecounter++;
    }
}


void TITLE_main(const DirectorCommand *curr_cmd, DirectorCommand *next_cmd)
{
    (void) curr_cmd; // ignore

    INPUT_set_cursor_visible(FALSE);

    _TITLE_disclaimer();

    // force palette to black
    u16 zeroes[64];
    memset(zeroes, 0x0000, 64*2);
    PAL_setColorsDMA(0, zeroes, 64);
    SYS_doVBlankProcess();

    // now load graphics in the peace of darkness
    u16 tilecnt = TILE_USER_INDEX;
    VDP_clearPlane(BG_A, TRUE);

    VDP_drawImageEx(BG_A, &title_dweep, TILE_ATTR_FULL(PAL1, 0, 0, 0, tilecnt), 12, -1, 0, DMA);
    tilecnt += title_dweep.tileset->numTile;
    VDP_loadTileSet(title_bg.tileset, tilecnt, DMA);
    PCTRL_set_source(PAL1, title_dweep.palette->data);

    Sprite * spr_ltr_dweep = SPR_addSprite(&title_letters_dweep, 60, 128, TILE_ATTR(PAL2, 1, 0, 0));
    Sprite * spr_ltr_genesis = SPR_addSprite(&title_letters_genesis, 72, 180, TILE_ATTR(PAL3, 1, 0, 0));
    PCTRL_set_source(PAL2, title_letters_dweep.palette->data);
    PCTRL_set_source(PAL3, title_letters_genesis.palette->data);

    for(u8 x = 0; x <= 20; x++)
        for(u8 y = 0; y <= 16; y++)
            VDP_setTileMapEx(BG_B, title_bg.tilemap, TILE_ATTR_FULL(PAL_LINE_HUD, 0, 0, 0, tilecnt), x*2, y*2, 0, 0, 2, 2, CPU);


    SFX_play(SFX_freeze);
    PCTRL_fade_in(PAL_STD_FADE_DURATION);
    u16 framecounter = 0;
    for(int i = 0; i < TITLE_FRAMES; i++)
    {
        VDP_setHorizontalScroll(BG_B, -((i/2)&0xf));
        VDP_setVerticalScroll(BG_B, (i/2)&0xf);
        SPR_update();
        PCTRL_step(framecounter);
        SYS_doVBlankProcess();
        framecounter++;

        if(framecounter == TITLE_FRAMES-PAL_STD_FADE_DURATION-1)
            PCTRL_fade_out(PAL_STD_FADE_DURATION);
    }

    SPR_releaseSprite(spr_ltr_dweep);
    SPR_releaseSprite(spr_ltr_genesis);

    memset(next_cmd, 0x00, sizeof(DirectorCommand));
    next_cmd->cmd = DIREC_CMD_LEVEL;

}

