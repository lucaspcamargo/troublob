#pragma once

#include <genesis.h>

#include "dweep_config.h"
#include "resources.h"

// HACK +256 for single bg tileset loaded at TILE_USER_INDEX, find better
#define TILE_HUD_INDEX (TILE_USER_INDEX+256)

static TileMap * hud_tm;

void HUD_draw();

void HUD_init()
{
    // set hud palette on its line
    PCTRL_set_source(PAL_LINE_HUD, pal_tset_hud.data, FALSE);

    // load HUD tiles
    VDP_loadTileSet(&tset_hud, TILE_HUD_INDEX, DMA);
    DMA_waitCompletion();

    // draw HUD
    HUD_draw();
}

void HUD_draw()
{
    // tiles on BG A
    VDP_setTileMapEx(BG_A, &map_hud, TILE_ATTR_FULL(PAL_LINE_HUD, 0, 0, 0, TILE_HUD_INDEX), 0, 24, 0, 0, 40, 4, DMA);
    // clear BG B behind
    VDP_fillTileMapRect(BG_B, 0, 0, 24, 40, 4);
    // wait till transfer done
    DMA_waitCompletion();
    //VDP_setTileMap(BG_A, &map_hud, 0, 24, 40, 4, CPU);
}


