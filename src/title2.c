#include <genesis.h>
#include "title2.h"

#include "dweep_config.h"
#include "resources.h"
#include "palette_ctrl.h"
#include "sfx.h"

#define TITLE_FRAMES 60

void TITLE_main()
{
    // immediately force palette to black
    u16 zeroes[64];
    memset(zeroes, 0x0000, 64*2);
    PAL_setColorsDMA(0, zeroes, 64);
    SYS_doVBlankProcess();

    // now load graphics in the peace of darkness
    u16 tilecnt = TILE_USER_INDEX;
    VDP_drawImageEx(BG_A, &title_dweep, TILE_ATTR_FULL(PAL1, 0, 0, 0, tilecnt), 1, 1, 0, DMA);
    tilecnt += title_dweep.tileset->numTile;
    VDP_drawImageEx(BG_A, &title_letters, TILE_ATTR_FULL(PAL1, 0, 0, 0, tilecnt), 13, 17, 0, DMA);
    tilecnt += title_letters.tileset->numTile;
    VDP_loadTileSet(title_bg.tileset, tilecnt, DMA);

    for(u8 x = 0; x <= 20; x++)
        for(u8 y = 0; y <= 16; y++)
            VDP_setTileMapEx(BG_B, title_bg.tilemap, TILE_ATTR_FULL(PAL1, 0, 0, 0, tilecnt), x*2, y*2, 0, 0, 2, 2, CPU);

    VDP_drawText(GAME_VERSION, 38-strlen(GAME_VERSION), 27);
    char xx[2];
    xx[0] = '0'+sizeof(void*);
    xx[1] = '\0';
    VDP_drawText(xx, 1, 27);

    PCTRL_set_source(1, title_dweep.palette->data, FALSE);
    PCTRL_set_source(2, title_letters.palette->data+16, FALSE);

    SFX_play(SFX_glass);
    PCTRL_fade_in(FADE_DUR);
    for(int i = 0; i < TITLE_FRAMES; i++)
    {
        VDP_setHorizontalScroll(BG_B, -((i/2)&0xf));
        VDP_setVerticalScroll(BG_B, (i/2)&0xf);
        SYS_doVBlankProcess();
    }

    PCTRL_fade_out(FADE_DUR);
    while(PAL_isDoingFade())
        SYS_doVBlankProcess();

}

