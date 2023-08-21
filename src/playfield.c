#include "playfield.h"
#include "palette_ctrl.h"
#include "registry.h"
#include "plf_obj.h"


enum PLFLaserFrame // description of laser sprite frames
{
    PLF_LASER_FRAME_H,
    PLF_LASER_FRAME_UL,
    PLF_LASER_FRAME_DL,
    PLF_LASER_FRAME_V,
    PLF_LASER_FRAME_DT,
    PLF_LASER_FRAME_DR,
    PLF_LASER_FRAME_UR,
    PLF_LASER_FRAME_LT,
} ENUM_PACK;


typedef struct MapObject_st {
    u16 type;
    u16 x;
    u16 y;
    u16 w;
    u16 h;
} MapObject;



// VARS
static u16 plf_curr_lvl_id;
static u16 plf_w;
static u16 plf_h;
static PlfTile plf_tiles[PLAYFIELD_STD_SZ];
static Map* m_a;
Map* m_b;
fix16 plf_cam_cx;
fix16 plf_cam_cy;
fix16 plf_player_initial_x;
fix16 plf_player_initial_y;

static u16 plf_vdp_tile_index_used;
static u16 ** plf_vdp_laser_tile_indices; // release this mem when playfield unloads?

#define TILE_AT(x, y) (plf_tiles[(x) + (y)*plf_w])

//private
void _PLF_do_create_object(const MapObject * obj, u16 x, u16 y);


void PLF_init(u16 lvl_id)
{
    plf_curr_lvl_id = lvl_id;
    const RGST_lvl curr_lvl = RGST_levels[plf_curr_lvl_id];

    // init playfield vars
    plf_w = PLAYFIELD_STD_W;
    plf_h = PLAYFIELD_STD_H;
    m_a = NULL;
    m_b = NULL;
    plf_cam_cx = FIX16(16*(PLAYFIELD_STD_W/2));
    plf_cam_cy = FIX16(16*(PLAYFIELD_STD_H/2));

    // initial graphics offset
    plf_vdp_tile_index_used = TILE_USER_INDEX;

    // setup palettes
    PCTRL_set_source(PAL_LINE_BG_0, curr_lvl.bg_tileset_pal->data, FALSE);
    PCTRL_set_source(PAL_LINE_BG_1, curr_lvl.bg_tileset_pal->data+16, FALSE);

    // TODO store palette ops with level descriptor?
    PalCtrlOperatorDescriptor pal_op =
    {
        PCTRL_OP_CYCLE,
        16*PAL_LINE_BG_0 + 30,
        2, 0x03
    };
    PCTRL_op_add(&pal_op);

    // load bg graphics
    VDP_loadTileSet(curr_lvl.bg_tileset, TILE_USER_INDEX, DMA);
    VDP_waitDMACompletion();
    plf_vdp_tile_index_used += curr_lvl.bg_tileset->numTile;

    // load laser graphics
    u16 loadedTiles;
    plf_vdp_laser_tile_indices = SPR_loadAllFrames(&spr_laser, plf_vdp_tile_index_used, &loadedTiles);
    plf_vdp_tile_index_used += loadedTiles;

    // load map data
    m_a = MAP_create(curr_lvl.bg_a_map, BG_A, TILE_ATTR_FULL(PAL_LINE_BG_0, 1, 0, 0, TILE_USER_INDEX));
    m_b = MAP_create(curr_lvl.bg_b_map, BG_B, TILE_ATTR_FULL(PAL_LINE_BG_0, 0, 0, 0, TILE_USER_INDEX));

    // init camera
    PLF_cam_to( FIX16(16*(PLAYFIELD_STD_W/2)),FIX16(16*(PLAYFIELD_STD_H/2)) );

    // init plane data
    PLF_update_scroll(TRUE);

    // clear field data
    memset(plf_tiles, 0x00, sizeof(plf_tiles));

    // process object definitions
    const void** const objs = curr_lvl.obj_map;
    const u16 obj_count = curr_lvl.obj_count;
    if(DEBUG_MAP_OBJS)
    {
        char buf[5];
        intToHex(obj_count, buf,4);
        VDP_drawTextBG(BG_A, buf, 0, 23);
        VDP_drawTextBG(BG_A, " OBJS", 4, 23);
    }

    Pobj_init();

    for(u16 iobj=0; iobj<obj_count; iobj++)
    {
        const MapObject* const obj = objs[iobj];
        if(obj->type >= PLF_OBJ_MAX)
            continue; // invalid obj spec

        if(obj->w || obj->h)
        {
            // evaluate object span
            u16 startx = obj->x/16;
            u16 endx = (obj->x+obj->w-1)/16;
            u16 starty;
            u16 endy;

            if(PLF_OBJ_IS_Y_BOTTOM(obj->type))
            {
                starty = (obj->y-obj->h)/16;
                endy = (obj->y-1)/16;
            }
            else{
                starty = obj->y/16;
                endy = (obj->y+obj->h-1)/16;
            }

            if(endy<starty || endx<startx)
                continue; // something overflowed or is negative

            if(endx >= plf_w || startx >= plf_w ||
               endy >= plf_h || starty >= plf_h )
                continue; // out of bounds

            bool single_pos = (startx==endx) && (starty==endy); // TODO use for single objects

            if(obj->type == PLF_OBJ_WALL)
            {
                for(u16 x = startx; x <= endx; x++)
                    for(u16 y = starty; y <= endy; y++)
                    {
                        TILE_AT(x,y).attrs |= PLF_ATTR_SOLID;
                    }
            } else if(single_pos)
            {
                if( obj->type == PLF_OBJ_PLAYER)
                {
                    plf_player_initial_x = FIX16(startx);
                    plf_player_initial_y = FIX16(starty);
                }
                else if( obj->type > PLF_OBJ_PLAYER)
                {
                    _PLF_do_create_object(obj, startx, starty);
                }
            }
        }
        else if(obj->type == PLF_OBJ_PLAYER)
        {
            // should be point object, jsut use x and y
            plf_player_initial_x = FIX16(obj->x/16);
            plf_player_initial_y = FIX16(obj->y/16);
        }

        if(DEBUG_MAP_OBJS && iobj < 28)
        {
            char buf[5];
            intToHex(obj->type,buf,2);
            VDP_drawTextBG(BG_A, buf, 0, iobj);
            VDP_drawTextBG(BG_B, buf, 0, iobj);
            intToHex(obj->x,buf,4);
            VDP_drawTextBG(BG_A, buf, 3, iobj);
            VDP_drawTextBG(BG_B, buf, 3, iobj);
            intToHex(obj->y,buf,4);
            VDP_drawTextBG(BG_A, buf, 8, iobj);
            VDP_drawTextBG(BG_B, buf, 8, iobj);
            intToHex(obj->w,buf,4);
            VDP_drawTextBG(BG_A, buf, 13, iobj);
            VDP_drawTextBG(BG_B, buf, 13, iobj);
            intToHex(obj->h,buf,4);
            VDP_drawTextBG(BG_A, buf, 18, iobj);
            VDP_drawTextBG(BG_B, buf, 18, iobj);
        }
    }

    if (DEBUG_TILES)
    {
        for(u16 x = 0; x < plf_w; x++)
            for(u16 y = 0; y < plf_h; y++)
            {
                // debug metatile using layer A
                char buf[4];
                intToHex(plf_tiles[x + y*plf_w].attrs,buf,2);
                VDP_drawTextBG(BG_A, buf, x*2, y*2);
            }
    }

    /* LASER SPRITE TEST
    for(int i = 0; i < 20; i++)
    {
        Sprite *s = SPR_addSprite(&spr_laser, i*16, 9*16 + PLF_LASER_Y_DELTA, TILE_ATTR_FULL(PAL_LINE_BG_1,0,0,0,0));
        SPR_setAutoTileUpload(s, FALSE);
        SPR_setVRAMTileIndex(s, plf_vdp_laser_tile_index);
        SPR_setAnimAndFrame(s, 0, 0);
    }*/
}

void _PLF_do_create_object(const MapObject* obj, u16 x, u16 y)
{
    PlfTile *t = PLF_get_tile(x, y);
    if(t->pobj)
        return; // object already in here, abort

    PobjEvtCreatedArgs args;
    args.plftile = t;
    u16 final_type = 0xffff;
    switch(obj->type)
    {
        case PLF_OBJ_WALL    :
        case PLF_OBJ_PLAYER  :
            return; // not handled here!
        case PLF_OBJ_GOAL    :
            final_type = POBJ_TYPE_GOAL;
            args.subtype = 0;
            break;
        case PLF_OBJ_LASER_R :
        case PLF_OBJ_LASER_L :
        case PLF_OBJ_LASER_U :
        case PLF_OBJ_LASER_D :
            final_type = POBJ_TYPE_LASER;
            args.subtype = obj->type - PLF_OBJ_LASER_R;
            break;
        case PLF_OBJ_MIRROR  :
        case PLF_OBJ_MIRROR2 :
            final_type = POBJ_TYPE_LASER;
            args.subtype = obj->type - DIR_R;
            break;
    }

    if(final_type == 0xffff)
        return; // type was not handled

    PobjHnd hnd = Pobj_alloc();
    if(hnd == POBJ_HND_INVAL)
        return; // invalid handle

    PobjData *dat = Pobj_get_data(hnd);
    dat->x = FIX16(x);
    dat->y = FIX16(y);
    dat->type = final_type;
    memset(dat->extra, 0, sizeof(dat->extra));
    Pobj_event(dat, POBJ_EVT_CREATED, &args);
}


void PLF_cam_to(fix16 cx, fix16 cy)
{
    plf_cam_cx = cx;
    plf_cam_cy = cy;
}

PlfTile * PLF_get_tile(u16 pf_x, u16 pf_y)
{
    return &plf_tiles[pf_x + pf_y*plf_w];
}

PlfTile * PLF_get_tile_safe(u16 pf_x, u16 pf_y)
{
    if(pf_x >= plf_w || pf_y >= plf_h)
        return NULL;
    return &plf_tiles[pf_x + pf_y*plf_w];
}

void PLF_player_get_initial_pos(fix16 *dest_x, fix16 *dest_y)
{
    *dest_x = plf_player_initial_x;
    *dest_y = plf_player_initial_y;
}

bool PLF_player_pathfind(u16 px, u16 py, u16 destx, u16 desty)
{
    // NOTE if/when scrolling levels are implemented, we'll need to do some coordinate conversion here
    //      and limit the pathfinding to the visible screen via stride_y
    //      for now, just convert to u8
    enum PathfindingResult res = PATH_find(PLAYFIELD_STD_W, PLAYFIELD_STD_H, (u8)px, (u8)py, (u8)destx, (u8)desty,
                                           ((u8*)&(plf_tiles->attrs)), sizeof(PlfTile), sizeof(PlfTile)*PLAYFIELD_STD_W, PLF_ATTR_SOLID);

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

/***
 * This function lets us add a laser to the playfield,
 * give the tile it is coming from, and the direction.
 * It walks along the laser path, creating the sprites for it,
 * and updating the tile data accordingly;.
 */
bool PLF_laser_put(u16 orig_x, u16 orig_y, u8 dir)
{
    PlfTile * tile = PLF_get_tile(orig_x, orig_y);
    const u16 OUTFLAG = PLF_LASER_OUT_R << dir;
    if(tile->laser & OUTFLAG)
        return FALSE; // laser coming out of here in this dir already

    tile->laser |= OUTFLAG;

    if( dir == DIR_D )
    {
        // in this special case, we start with graphics placement
        // CONSIDER: maybe just embed this into the canon graphics?
        Sprite *s = SPR_addSprite(&spr_laser, orig_x*16, orig_y*16, TILE_ATTR_FULL(PAL_LINE_BG_1,0,0,0,0));
        SPR_setAutoTileUpload(s, FALSE);
        SPR_setAnimAndFrame(s, 0, 4);
        SPR_setVRAMTileIndex(s, plf_vdp_laser_tile_indices[0][4]);
        SPR_setDepth(s, PLF_get_sprite_depth(FIX16(orig_x), FIX16(orig_y)) - 1); // <-- note the minus one for laser
    }

    u16 curr_x = orig_x;
    u16 curr_y = orig_y;
    bool terminated = FALSE;

    while(!terminated)
    {
        // update x and y of next in
        DIR_UPDATE_XY(curr_x, curr_y, dir);
        tile = PLF_get_tile_safe(curr_x, curr_y);
        if(!tile)
        {
            // abort early if
            break;
        }

        tile->laser &= (PLF_LASER_IN_R << dir);
        u8 laser_frame = DIR_IS_HORIZONTAL(dir)? PLF_LASER_FRAME_H : PLF_LASER_FRAME_V;
        bool flip_laser_frame = FALSE;

        if(tile->pobj)
        {
            // TODO do laser query, etc
            // HINT: update dir and laser frame if mirror
        }
        else if(tile->attrs & PLF_ATTR_SOLID)
        {
            terminated = TRUE;
            switch(dir)
            {
                case DIR_L:
                    flip_laser_frame = TRUE;
                case DIR_R:
                    laser_frame = PLF_LASER_FRAME_LT;
                    break;
                case DIR_U:
                    laser_frame = PLF_LASER_FRAME_DT;
                    break;
                case DIR_D:
                    laser_frame = 0xff;
            }
        }
        else
        {
            // no object, not solid
        }

        if(!terminated)
            tile->laser &= (PLF_LASER_OUT_R << dir);

        if(laser_frame != 0xff)
        {
            // TODO own this sprite somewhere somehow
            Sprite *s = SPR_addSprite(&spr_laser, curr_x*16, curr_y*16, TILE_ATTR_FULL(PAL_LINE_BG_1,0,0,0,0));
            SPR_setAutoTileUpload(s, FALSE);
            SPR_setAnimAndFrame(s, 0, laser_frame);
            SPR_setVRAMTileIndex(s, plf_vdp_laser_tile_indices[0][laser_frame]);
            SPR_setDepth(s, PLF_get_sprite_depth(FIX16(curr_x), FIX16(curr_y)) + 1); // <-- note the plus one for laser
            if(flip_laser_frame)
            {
                if(DIR_IS_HORIZONTAL(dir))
                    SPR_setHFlip(s, TRUE);
                else
                    SPR_setVFlip(s, TRUE);
            }
        }
    }

    // NOTE: think, do something else here, maybe?

    return TRUE;
}

/*
 * Note: forceRedraw will call vblank processing twice for uploading map data to planes
 * */
void PLF_update_scroll(bool forceRedraw)
{
    s16 plf_scroll_x = fix16ToRoundedInt(fix16Sub(plf_cam_cx, FIX16(160)));
    s16 plf_scroll_y = fix16ToRoundedInt(fix16Sub(plf_cam_cy, FIX16(96)));

    //VDP_setHorizontalScroll(BG_A, -plf_scroll_x);
    //VDP_setHorizontalScroll(BG_B, -plf_scroll_x);
    //VDP_setVerticalScroll(BG_A, plf_scroll_y);
    //VDP_setVerticalScroll(BG_B, plf_scroll_y);

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
        VDP_drawTextBG(WINDOW, buf, 0, 27);
    }
}

