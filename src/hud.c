#include "hud.h"
#include "palette_ctrl.h"

void HUD_init()
{
    // set hud palette on its line
    PCTRL_set_source(PAL_LINE_HUD, pal_tset_hud.data, FALSE);

    // load HUD tiles
    VDP_loadTileSet(&tset_hud, TILE_HUD_INDEX, DMA);

    // draw HUD
    HUD_draw();
}

void HUD_draw()
{
    VDP_setWindowVPos(TRUE, 20);
    // tiles on BG A
    VDP_setTileMapEx(WINDOW, &map_hud, TILE_ATTR_FULL(PAL_LINE_HUD, 0, 0, 0, TILE_HUD_INDEX), 0, 24, 0, 0, 40, 4, DMA);
    // clear BG B behind -- disabled
    //VDP_fillTileMapRect(BG_B, 0, 0, 24, 40, 4);
    // wait till transfer done
    DMA_waitCompletion();
    //VDP_setTileMap(BG_A, &map_hud, 0, 24, 40, 4, CPU);
    //VDP_drawTextBG(WINDOW, "MOVE", 32 - 24, 26);
}

