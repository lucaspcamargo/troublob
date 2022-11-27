#pragma once

#include <genesis.h>

#include "dweep_config.h"
#include "resources.h"
#include "pathfind.h"

// TYPE DFINITIONS

// uniform attributes for every playfield tile
enum PlfAttrBits {
    PLF_ATTR_SOLID = 1 << 0,    // is solid
    PLF_ATTR_HOT   = 1 << 1,    // is hot plate
    PLF_ATTR_COLD  = 1 << 2,    // is cold plate
    PLF_ATTR_OBJ   = 1 << 3,    // contains object, can be solid or not
    PLF_ATTR_DIR_FLIP = 1 << 4, // for every flippable object (incl mirrors)
    PLF_ATTR_DIR_VERT = 1 << 5, // four-way objects like lasers and fans, stacks with _FLIP
    PLF_ATTR_EXTRA_A  = 1 << 6, // reserved
    PLF_ATTR_EXTRA_B = 1 << 7   // reserved
};

// laser attributes for every playfield tile
// for every quadrant direction we can have lasers in and out of the tile
enum PlfLaserBits {
    PLF_LASER_IN_R  = 1 << 0,
    PLF_LASER_IN_L  = 1 << 1,
    PLF_LASER_IN_U  = 1 << 2,
    PLF_LASER_IN_D  = 1 << 3,
    PLF_LASER_OUT_R = 1 << 4,
    PLF_LASER_OUT_L = 1 << 5,
    PLF_LASER_OUT_U = 1 << 6,
    PLF_LASER_OUT_D = 1 << 7
};

typedef struct {
    u8 ident;
    u8 attrs;
    u8 laser;
} PlfTile;

static const u8 tileset_attr_mapping[] = {
    /*0: Wall (covered) */ PLF_ATTR_SOLID,
    /*1: Wall (uncovered) */ PLF_ATTR_SOLID,
    /*2: Ice  */ PLF_ATTR_COLD,
    /*3: Broken_Machine */ NULL,
    /*4: Ground */ 0,
    /*5: Pentagram */ 0,
    /*6: Lava */ PLF_ATTR_HOT,
    /*7: Item */ PLF_ATTR_OBJ,
    /*8: ??? */ NULL,
    /*9: Laser */ PLF_ATTR_OBJ|PLF_ATTR_SOLID,
    /*10: Ground */ 0
};

// VAR DEFINITIONS

u16 plf_w = PLAYFIELD_STD_W;
u16 plf_h = PLAYFIELD_STD_H;
PlfTile plf_tiles[PLAYFIELD_STD_SZ];

Map* m_a;
Map* m_b;

fix16 plf_cam_cx = FIX16(16*(PLAYFIELD_STD_W/2));
fix16 plf_cam_cy = FIX16(16*(PLAYFIELD_STD_H/2));


// fwd decls
void PLF_update_scroll(bool forceRedraw);
void PLF_cam_to(fix16 cx, fix16 cy);


void PLF_init()
{
    // setup palette
    PCTRL_set_source(PAL_LINE_BG_0, pal_tset_0.data, FALSE);
    PCTRL_set_source(PAL_LINE_BG_1, pal_tset_0.data+(pal_tset_0.length/4), FALSE);


    // TODO store these palette ops with tileset somehow?
    PalCtrlOperatorDescriptor pal_op =
    {
        PCTRL_OP_CYCLE,
        44, 3, 0x1f, NULL, NULL
    };
    PCTRL_op_add(&pal_op);
    pal_op.idx_base = 62;
    pal_op.len = 2;
    pal_op.period_mask = 0x03;
    PCTRL_op_add(&pal_op);


    // load bg graphics
    VDP_loadTileSet(&tset_0, TILE_USER_INDEX, DMA);

    // load map data
    m_a = MAP_create(&map_2_a, BG_A, TILE_ATTR_FULL(PAL_LINE_BG_0, 1, 0, 0, TILE_USER_INDEX));
    m_b = MAP_create(&map_2_b, BG_B, TILE_ATTR_FULL(PAL_LINE_BG_0, 0, 0, 0, TILE_USER_INDEX));

    // init camera
    PLF_cam_to( FIX16(16*(PLAYFIELD_STD_W/2)),FIX16(16*(PLAYFIELD_STD_H/2)) );

    // init plane data
    PLF_update_scroll(TRUE);

    // convert plane data into field data
    for(u16 x = 0; x < plf_w; x++)
        for(u16 y = 0; y < plf_h; y++)
        {
            u16 meta = MAP_getMetaTile(m_b,x,y);
            u8 tileAttr = (meta<sizeof(tileset_attr_mapping))?
                    tileset_attr_mapping[meta] :
                    0xFF; // not found
            PlfTile tile;
            tile.ident = meta<<4;
            tile.attrs = tileAttr==0xFF? 0 : tileAttr;
            tile.laser = 0;
            plf_tiles[x + y*plf_w] = tile;
            if (DEBUG_METATILES)
            {
                // debug metatile using layer A:
                VDP_setTileMapXY(BG_A, TILE_ATTR_FULL(PAL0,0,0,0,meta+(meta>9?0x05C1-10:0x05B0)), x*2, y*2);
                if(tileAttr != 0xFF)
                    VDP_setTileMapXY(BG_A, TILE_ATTR_FULL(PAL0,0,0,0,tileAttr+(tileAttr>9?0x05C1-10:0x05B0)), x*2+1, y*2);
            }
        }
}

void PLF_cam_to(fix16 cx, fix16 cy)
{
    plf_cam_cx = cx;
    plf_cam_cy = cy;
}

PlfTile PLF_get_tile(u16 pf_x, u16 pf_y)
{
    return plf_tiles[pf_x + pf_y*plf_w];
}

bool PLF_player_pathfind(u16 px, u16 py, u16 destx, u16 desty)
{
    // NOTE if/when scrolling levels are implemented, we'll need to do some coordinate conversion here
    //      and limit the pathfinding to the current screen via stride_y
    //      for now, just convert to u8
    enum PathfindingResult res = PATH_find(PLAYFIELD_STD_W, PLAYFIELD_STD_H, (u8)px, (u8)py, (u8)destx, (u8)desty,
                                           ((u8*)plf_tiles)+1, sizeof(PlfTile), sizeof(PlfTile)*PLAYFIELD_STD_W, PLF_ATTR_SOLID);

    if(DEBUG_PATHFINDING)
    {
        char buf[16] = "PATH RES ";
        intToHex(res, buf+9, 1);
        VDP_drawText(buf, 20, 27);
    }
    return res==PATH_FOUND;
}

/*
 * Note: forceRedraw will call vblank processing twice for uploading map data to planes
 * */
void PLF_update_scroll(bool forceRedraw)
{
    s16 plf_scroll_x = fix16ToRoundedInt(fix16Sub(plf_cam_cx, FIX16(160)));
    s16 plf_scroll_y = fix16ToRoundedInt(fix16Sub(plf_cam_cy, FIX16(96)));

    VDP_setHorizontalScroll(BG_A, -plf_scroll_x);
    VDP_setHorizontalScroll(BG_B, -plf_scroll_x);
    VDP_setVerticalScroll(BG_A, plf_scroll_y);
    VDP_setVerticalScroll(BG_B, plf_scroll_y);

    MAP_scrollToEx(m_a, plf_scroll_x, plf_scroll_y, forceRedraw);
    if (forceRedraw)
        SYS_doVBlankProcess();
    MAP_scrollToEx(m_b, plf_scroll_x, plf_scroll_y, forceRedraw);
    if (forceRedraw)
        SYS_doVBlankProcess();

    if(DEBUG_CAMERA)
    {
        char buf[40];
        sprintf(buf, "@%d,%d   ", fix16ToRoundedInt(plf_cam_cx), fix16ToRoundedInt(plf_cam_cy));
        VDP_drawTextBG(BG_A, buf, 0, 27);
    }
}
