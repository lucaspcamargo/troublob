#include "playfield.h"
#include "palette_ctrl.h"


// TODO metatile indices seem to be stored in map according to how they are shown on screen
//      the actual tilemap does not matter for metatile index
//      need to be able to get metatile information per the tileset it came from

static const u8 tileset_attr_mapping[] = {  // FOR MAP 1
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

//static const u8 tileset_attr_mapping[] = {  // FOR MAP 3
//    PLF_ATTR_SOLID,
//    0,
//    PLF_ATTR_SOLID,
//    NULL,
//    0,
//    0,
//    PLF_ATTR_HOT,
//    PLF_ATTR_OBJ,
//    NULL,
//    PLF_ATTR_OBJ|PLF_ATTR_SOLID,
//    0
//};

// VARS
u16 plf_w;
u16 plf_h;
PlfTile plf_tiles[PLAYFIELD_STD_SZ];
Map* m_a;
Map* m_b;
fix16 plf_cam_cx;
fix16 plf_cam_cy;

void PLF_init()
{
    // init playfield vars
    plf_w = PLAYFIELD_STD_W;
    plf_h = PLAYFIELD_STD_H;
    m_a = NULL;
    m_b = NULL;
    plf_cam_cx = FIX16(16*(PLAYFIELD_STD_W/2));
    plf_cam_cy = FIX16(16*(PLAYFIELD_STD_H/2));

    // setup palettes
    PCTRL_set_source(PAL_LINE_BG_0, pal_tset_0.data, FALSE);
    PCTRL_set_source(PAL_LINE_BG_1, pal_tset_0.data+16, FALSE);

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
    m_a = MAP_create(&map_1_a, BG_A, TILE_ATTR_FULL(PAL_LINE_BG_0, 1, 0, 0, TILE_USER_INDEX));
    m_b = MAP_create(&map_1_b, BG_B, TILE_ATTR_FULL(PAL_LINE_BG_0, 0, 0, 0, TILE_USER_INDEX));

    // init camera
    PLF_cam_to( FIX16(16*(PLAYFIELD_STD_W/2)),FIX16(16*(PLAYFIELD_STD_H/2)) );

    // init plane data
    PLF_update_scroll(TRUE);

    // convert plane data into field data
    for(u16 x = 0; x < plf_w; x++)
        for(u16 y = 0; y < plf_h; y++)
        {
            u16 metaIdx = MAP_getMetaTile(m_b,x,y) & 0x07FF;
            u8 tileAttr = (metaIdx<sizeof(tileset_attr_mapping))?
                    tileset_attr_mapping[metaIdx] :
                    0xFF; // not found
            PlfTile tile;
            tile.ident = metaIdx<<4;
            tile.attrs = tileAttr==0xFF? 0 : tileAttr;
            tile.laser = 0;
            plf_tiles[x + y*plf_w] = tile;
            if (DEBUG_TILES)
            {
                // debug metatile using layer A:
                //VDP_setTileMapXY(BG_A, TILE_ATTR_FULL(PAL0,0,0,0,meta+(meta>9?0x05C1-10:0x05B0)), x*2, y*2);
                char buf[4];
                intToHex(metaIdx,buf,2);
                VDP_drawTextBG(BG_A, buf, x*2, y*2);
                //if(tileAttr != 0xFF)
                    //VDP_setTileMapXY(BG_A, TILE_ATTR_FULL(PAL0,0,0,0,tileAttr+(tileAttr>9?0x05C1-10:0x05B0)), x*2+1, y*2);
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
        char buf[20];
        sprintf(buf, "%d,%d to %d,%d: %d(%d)", (int) px, (int) py, (int) destx, (int) desty, (int) res, (int) PATH_curr_node_count() );
        VDP_drawText(buf, 16, 27);
    }
    return res==PATH_FOUND;
}

bool PLF_player_path_next(u16 px, u16 py, u16 *nextx, u16 *nexty)
{
    u8 bufx, bufy;
    bool ret = PATH_next(PLAYFIELD_STD_W, PLAYFIELD_STD_H, (u8)px, (u8)py, &bufx, &bufy);
    if(ret)
    {
        *nextx = bufx;
        *nexty = bufy;
    }
    return ret;
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
