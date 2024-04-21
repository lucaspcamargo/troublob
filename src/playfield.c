/*
This file is part of Dweep Genesis.

Dweep Genesis is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

Dweep Genesis is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with Foobar. If not, see <https://www.gnu.org/licenses/>.
*/

#include "playfield.h"
#include "dweep_globals.h"
#include "palette_ctrl.h"
#include "registry.h"
#include "plf_obj.h"
#include "gfx_utils.h"

// DEBUG
#include "sfx.h"


enum PlfLaserFrame // description of laser sprite frames
{
    PLF_LASER_FRAME_H,
    PLF_LASER_FRAME_UL,
    PLF_LASER_FRAME_DL,
    PLF_LASER_FRAME_V,
    PLF_LASER_FRAME_DT,
    PLF_LASER_FRAME_DR,
    PLF_LASER_FRAME_UR,
    PLF_LASER_FRAME_LT,
    PLF_LASER_FRAME_CROSS,
    PLF_LASER_FRAME_RT,
    PLF_LASER_FRAME_NONE = 0xff
} ENUM_PACK;

enum PlfLaserGfx
{
    PLF_LASER_GFX_NONE = 0,
    PLF_LASER_GFX_H = 1,
    PLF_LASER_GFX_V = 2,
    PLF_LASER_GFX_QUAD_UL = 4, // top-left
    PLF_LASER_GFX_QUAD_UR = 8, // top-right
    PLF_LASER_GFX_QUAD_DL = 16, // bottom-left
    PLF_LASER_GFX_QUAD_DR = 32, // bottom-right
    PLF_LASER_GFX_TERM_D = 64, // bottom termination
    PLF_LASER_GFX_TERM_R = 128, // right termination
    PLF_LASER_GFX_TERM_L = 256, // left termination
    PLF_LASER_GFX_HV = 512, // crossed lasers

    // utility values
    PLF_LASER_GFX_QUAD_ANY = 0b111100   // UL|UR|DL|DR
} ENUM_PACK;


typedef struct MapObject_st {
    u16 type;
    u16 x;
    u16 y;
    u16 w;
    u16 h;
} MapObject;

typedef struct PlfLaserPutTicket_st
{
    u8 used;
    u8 dir;
    u16 x;
    u16 y;
} PlfLaserPutTicket;

typedef struct PlfLaserSprEntry_st {
    Sprite *spr;
    u8 px;
    u8 py;
} PlfLaserSprEntry;


#define PLF_LASER_PUT_TICKET_MAX 32
#define PLF_LASER_SPRITE_MAX 32

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
u16 plf_orig_tile_watermark;

static const u8 * plf_plane_a_alloc;
static u16 plf_plane_a_alloc_stride;

static const SpriteDefinition* plf_theme_sprite_defs[PLF_THEME_COUNT];
static u16 ** plf_theme_tile_indices[PLF_THEME_COUNT];
static u8     plf_theme_palette_line[PLF_THEME_COUNT];

static PlfLaserSprEntry plf_laser_sprites[PLF_LASER_SPRITE_MAX];
static bool plf_laser_put_defer; // on playfield init
static PlfLaserPutTicket plf_laser_put_tickets[PLF_LASER_PUT_TICKET_MAX];
static u8 plf_laser_put_tickets_mark;


#define TILE_AT(x, y) (plf_tiles[(x) + (y)*plf_w])
#define PLANE_A_ALLOCATION_AT(x, y) (plf_plane_a_alloc[(y)*plf_plane_a_alloc_stride + (x)/8]&(1<<((x)%8)))

//private
void _PLF_load_theme();
void _PLF_load_objects();
void _PLF_init_create_object(const MapObject * obj, u16 x, u16 y);
void _PLF_laser_gfx_update(u16 x, u16 y, bool force_sprite);
inline enum PlfLaserGfx _PLF_laser_in_out_to_gfx(u8 in_dir, u8 out_dir);
inline enum PlfLaserFrame _PLF_laser_gfx_to_frame(enum PlfLaserGfx gfx);
inline u8 _PLF_laser_behavior_apply(u8 behavior, u8 in_dir);
PlfLaserSprEntry *_PLF_laser_spr_find_free(u8 *start);
PlfLaserSprEntry *_PLF_laser_spr_find_first(u8 x, u8 y, u8 *start);
bool _PLF_laser_spr_setup(PlfLaserSprEntry * entry, u8 x, u8 y, enum PlfLaserFrame frame, bool behind);
void _PLF_laser_spr_free(PlfLaserSprEntry * entry);


void PLF_init(u16 lvl_id)
{
    plf_curr_lvl_id = lvl_id;
    plf_orig_tile_watermark = GLOBAL_vdp_tile_watermark;
    const RGST_lvl curr_lvl = RGST_levels[plf_curr_lvl_id];

    // init playfield vars
    plf_w = PLAYFIELD_VIEW_W; // curr_lvl.bg_b_map->w; not working???? TODO DEBUG THIS
    plf_h = PLAYFIELD_VIEW_H;
    m_a = NULL;
    m_b = NULL;

    plf_plane_a_alloc = curr_lvl.bg_a_allocation;
    plf_plane_a_alloc_stride = plf_plane_a_alloc? plf_w/8 + (plf_w%8? 1 : 0) : 0xffff;

    // setup palettes
    PCTRL_set_line(PAL_LINE_BG_0, curr_lvl.bg_tileset_pal->data);
    PCTRL_set_line(PAL_LINE_BG_1, curr_lvl.bg_tileset_pal->data+16);

    PCTRL_op_clear_all();
    for(int p = 0; p < RGST_PCTRL_OP_MAX; p++)
    {
        if(curr_lvl.pal_ops[p].operation != PCTRL_OP_NOOP)
            PCTRL_op_add(&curr_lvl.pal_ops[p]);
    }

    // load bg graphics
    VDP_loadTileSet(curr_lvl.bg_tileset, GLOBAL_vdp_tile_watermark, DMA);
    VDP_waitFIFOEmpty();
    GLOBAL_vdp_tile_watermark += curr_lvl.bg_tileset->numTile;

    // load map data
    m_a = MAP_create(curr_lvl.bg_a_map, BG_A, TILE_ATTR_FULL(PAL_LINE_BG_0, 1, 0, 0, TILE_USER_INDEX));
    m_b = MAP_create(curr_lvl.bg_b_map, BG_B, TILE_ATTR_FULL(PAL_LINE_BG_0, 0, 0, 0, TILE_USER_INDEX));

    // init camera
    PLF_cam_to( FIX16(16*(PLAYFIELD_VIEW_W/2)),FIX16(16*(PLAYFIELD_VIEW_H/2-1)) );

    // init plane data
    PLF_update_scroll(TRUE);

    // clear field data
    memset(plf_tiles, 0x00, sizeof(plf_tiles));
    memset(plf_laser_sprites, 0x00, sizeof(plf_laser_sprites));

    _PLF_load_theme();
    _PLF_load_objects();

}


void PLF_reset(u16 lvl_id)
{
    PLF_destroy();
    PLF_init(lvl_id);
}


void PLF_destroy()
{
    GLOBAL_vdp_tile_watermark = plf_orig_tile_watermark;
    bool reset = TRUE;
    Pobj_event_to_all(POBJ_EVT_DESTROYED, &reset, 0);
    Pobj_destroy();
    for(u8 i = 0; i < PLF_LASER_SPRITE_MAX; i++)
        if(plf_laser_sprites[i].spr)
            SPR_releaseSprite(plf_laser_sprites[i].spr);
    free(m_a);
    free(m_b);
    m_a = m_b = NULL;
}


void _PLF_load_theme()
{
    // TODO in future, if needed, allow these to vary with level entry
    // maybe create a theme structure that can be referred to by the level

    u16 loadedTiles;
    memset(plf_theme_sprite_defs, 0, sizeof(plf_theme_sprite_defs));
    memset(plf_theme_tile_indices, 0, sizeof(plf_theme_tile_indices));
    memset(plf_theme_palette_line, 0, sizeof(plf_theme_palette_line));

    plf_theme_sprite_defs[PLF_THEME_LASER_LIGHT] = &spr_laser;
    plf_theme_palette_line[PLF_THEME_LASER_LIGHT] = PAL_LINE_BG_1;
    plf_theme_tile_indices[PLF_THEME_LASER_LIGHT] = SPR_loadAllFrames(&spr_laser, GLOBAL_vdp_tile_watermark, &loadedTiles);
    GLOBAL_vdp_tile_watermark += loadedTiles;

    plf_theme_sprite_defs[PLF_THEME_LASER_CANNON] = &spr_laser_cannon;
    plf_theme_palette_line[PLF_THEME_LASER_CANNON] = PAL_LINE_SPR_A;
    plf_theme_tile_indices[PLF_THEME_LASER_CANNON] = SPR_loadAllFrames(&spr_laser_cannon, GLOBAL_vdp_tile_watermark, &loadedTiles);
    GLOBAL_vdp_tile_watermark += loadedTiles;

    plf_theme_sprite_defs[PLF_THEME_MIRROR] = &spr_mirror;
    plf_theme_palette_line[PLF_THEME_MIRROR] = PAL_LINE_SPR_A;
    plf_theme_tile_indices[PLF_THEME_MIRROR] = SPR_loadAllFrames(&spr_mirror, GLOBAL_vdp_tile_watermark, &loadedTiles);
    GLOBAL_vdp_tile_watermark += loadedTiles;

    plf_theme_sprite_defs[PLF_THEME_TOOLS] = &spr_tools;
    plf_theme_palette_line[PLF_THEME_TOOLS] = PAL_LINE_SPR_A;
    plf_theme_tile_indices[PLF_THEME_TOOLS] = SPR_loadAllFrames(&spr_tools, GLOBAL_vdp_tile_watermark, &loadedTiles);
    GLOBAL_vdp_tile_watermark += loadedTiles;

    plf_theme_sprite_defs[PLF_THEME_GOAL] = &spr_goal_plate;
    plf_theme_palette_line[PLF_THEME_GOAL] = PAL_LINE_SPR_A;
    plf_theme_tile_indices[PLF_THEME_GOAL] = SPR_loadAllFrames(&spr_goal_plate, GLOBAL_vdp_tile_watermark, &loadedTiles);
    GLOBAL_vdp_tile_watermark += loadedTiles;

    plf_theme_sprite_defs[PLF_THEME_GOAL_BABIES] = &spr_goal_babies;
    plf_theme_palette_line[PLF_THEME_GOAL_BABIES] = PAL_LINE_SPR_A;
    plf_theme_tile_indices[PLF_THEME_GOAL_BABIES] = SPR_loadAllFrames(&spr_goal_babies, GLOBAL_vdp_tile_watermark, &loadedTiles);
    GLOBAL_vdp_tile_watermark += loadedTiles;

    plf_theme_sprite_defs[PLF_THEME_BOMB] = &spr_bomb;
    plf_theme_palette_line[PLF_THEME_BOMB] = PAL_LINE_SPR_A;
    plf_theme_tile_indices[PLF_THEME_BOMB] = SPR_loadAllFrames(&spr_bomb, GLOBAL_vdp_tile_watermark, &loadedTiles);
    GLOBAL_vdp_tile_watermark += loadedTiles;

    plf_theme_sprite_defs[PLF_THEME_HOT] = &spr_plate_hot;
    plf_theme_palette_line[PLF_THEME_HOT] = PAL_LINE_SPR_A;
    plf_theme_tile_indices[PLF_THEME_HOT] = SPR_loadAllFrames(&spr_plate_hot, GLOBAL_vdp_tile_watermark, &loadedTiles);
    GLOBAL_vdp_tile_watermark += loadedTiles;

    plf_theme_sprite_defs[PLF_THEME_COLD] = &spr_plate_cold;
    plf_theme_palette_line[PLF_THEME_COLD] = PAL_LINE_SPR_A;
    plf_theme_tile_indices[PLF_THEME_COLD] = SPR_loadAllFrames(&spr_plate_cold, GLOBAL_vdp_tile_watermark, &loadedTiles);
    GLOBAL_vdp_tile_watermark += loadedTiles;

    plf_theme_sprite_defs[PLF_THEME_EXPLOSION] = &spr_explosion;
    plf_theme_palette_line[PLF_THEME_EXPLOSION] = PAL_LINE_SPR_A;
    plf_theme_tile_indices[PLF_THEME_EXPLOSION] = SPR_loadAllFrames(&spr_explosion, GLOBAL_vdp_tile_watermark, &loadedTiles);
    GLOBAL_vdp_tile_watermark += loadedTiles;
}


void _PLF_load_objects()
{
    const RGST_lvl curr_lvl = RGST_levels[plf_curr_lvl_id];

    plf_laser_put_defer = TRUE;
    plf_laser_put_tickets_mark = 0;
    memset(&plf_laser_put_tickets, 0x00, sizeof(plf_laser_put_tickets));

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

            bool single_pos = (startx==endx) && (starty==endy);

            if(obj->type == PLF_OBJ_WALL)
            {
                for(u16 x = startx; x <= endx; x++)
                    for(u16 y = starty; y <= endy; y++)
                    {
                        TILE_AT(x,y).attrs |= (PLF_ATTR_PLAYER_SOLID | PLF_ATTR_LASER_SOLID);
                    }
            } else if(obj->type == PLF_OBJ_HOLE)
            {
                for(u16 x = startx; x <= endx; x++)
                    for(u16 y = starty; y <= endy; y++)
                    {
                        TILE_AT(x,y).attrs |= (PLF_ATTR_HOLE);
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
                    _PLF_init_create_object(obj, startx, starty);
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

    plf_laser_put_defer = FALSE;
    for(u8 i = 0; i < PLF_LASER_PUT_TICKET_MAX; i++)
    {
        if(plf_laser_put_tickets[i].used)
            PLF_laser_put(plf_laser_put_tickets[i].x, plf_laser_put_tickets[i].y, plf_laser_put_tickets[i].dir);
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
        VDP_waitFIFOEmpty();
    }


    if(DEBUG_MAP_PLANE_A_ALLOC && curr_lvl.bg_a_allocation)
    {
        u16 stride = plf_w/8 + (plf_w%8? 1 : 0);
        for(u16 x = 0; x < plf_w; x++)
            for(u16 y = 0; y < plf_h; y++)
            {
                u8 mask = 1 << (x%8);
                if(curr_lvl.bg_a_allocation[y*stride + x/8]&mask)
                    VDP_drawTextBG(BG_A, "X", x*2, y*2);
            }
        VDP_waitFIFOEmpty();
    }
}

void _PLF_init_create_object(const MapObject* obj, u16 x, u16 y)
{
    PlfTile *t = PLF_get_tile(x, y);
    if(t->pobj)
        return; // object already in here, abort

    PobjEvtCreatedArgs args;
    args.plftile = t;
    args.subtype = 0;
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
        case PLF_OBJ_MIRROR2 :
            args.subtype = 1;
        case PLF_OBJ_MIRROR  :
            final_type = POBJ_TYPE_MIRROR;
            break;
        case PLF_OBJ_BOMB:
            final_type = POBJ_TYPE_BOMB;
            break;
        case PLF_OBJ_HEAT:
            final_type = POBJ_TYPE_HEAT;
            break;
        case PLF_OBJ_COLD:
            final_type = POBJ_TYPE_COLD;
            break;
    }

    if(obj->type >= PLF_OBJ_ITEM_BASE)
    {
        final_type = POBJ_TYPE_TOOL_ITEM;
        args.subtype = obj->type-PLF_OBJ_ITEM_BASE;
    }

    if(final_type == 0xffff)
        return; // type was not handled

    PobjHnd hnd = Pobj_alloc();
    if(hnd == POBJ_HND_INVAL)
        return; // invalid handle

    t->pobj = hnd;

    PobjData *dat = Pobj_get_data(hnd);
    dat->x = FIX16(x);
    dat->y = FIX16(y);
    dat->type = final_type;
    memset(dat->extra, 0, sizeof(dat->extra));
    Pobj_event(hnd, POBJ_EVT_CREATED, &args);
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
    enum PathfindingResult res = PATH_find(PLAYFIELD_VIEW_W, PLAYFIELD_VIEW_H, (u8)px, (u8)py, (u8)destx, (u8)desty,
                                           ((u8*)&(plf_tiles->attrs)), sizeof(PlfTile), sizeof(PlfTile)*plf_w, PLF_ATTR_PLAYER_SOLID | PLF_ATTR_HOLE);

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
    bool ret = PATH_next(PLAYFIELD_VIEW_W, PLAYFIELD_VIEW_H, (u8)px, (u8)py, &bufx, &bufy);
    if(ret)
    {
        *nextx = bufx;
        *nexty = bufy;
    }
    return ret;
}


void* PLF_obj_create(u16 pobj_type, u16 pobj_subtype, u16 px, u16 py)
{
    PlfTile * t = PLF_get_tile_safe(px, py);
    if(!t)
        return NULL;

    PobjHnd hnd = Pobj_alloc();
    if(hnd == POBJ_HND_INVAL)
        return NULL;
    t->pobj = hnd;

    PobjData *dat = Pobj_get_data(hnd);
    dat->x = FIX16(px);
    dat->y = FIX16(py);
    dat->type = pobj_type;
    memset(dat->extra, 0, sizeof(dat->extra));
    PobjEvtCreatedArgs args;
    memset(&args, 0, sizeof(PobjEvtCreatedArgs));
    args.subtype = pobj_subtype;
    args.plftile = t;
    Pobj_event(dat, POBJ_EVT_CREATED, &args);
    return hnd;
}

void* PLF_obj_at(u16 px, u16 py)
{
    const PlfTile * t = PLF_get_tile_safe(px, py);
    if(!t)
        return NULL;

    return t->pobj;
}

void PLF_obj_destroy(u16 px, u16 py, void *evt_arg)
{
    PlfTile * t = PLF_get_tile_safe(px, py);
    if(!t)
        return;

    if(t->pobj)
    {
        Pobj_event(Pobj_get_data(t->pobj), POBJ_EVT_DESTROYED, evt_arg);
        Pobj_dealloc(t->pobj);
        t->pobj = NULL;
    }
}


void  PLF_obj_damage(u8 type, u16 px, u16 py)
{
    PlfTile * t = PLF_get_tile_safe(px, py);
    if(!t)
        return;

    if(t->pobj)
    {
        Pobj_event(Pobj_get_data(t->pobj), POBJ_EVT_DAMAGE, &type);
    }

}

/***
 * This function lets us add a laser to the playfield,
 * give the tile it is coming from, and the direction.
 * It walks along the laser path, creating the sprites for it,
 * and updating the tile data accordingly;.
 */
bool PLF_laser_put(u16 orig_x, u16 orig_y, u8 dir)
{
    if(plf_laser_put_defer)
    {
        for(u8 i = 0; i < PLF_LASER_PUT_TICKET_MAX; i++)
        {
            u8 curr = (plf_laser_put_tickets_mark + 1 + i) % PLF_LASER_PUT_TICKET_MAX;
            if(!plf_laser_put_tickets[i].used)
            {
                plf_laser_put_tickets[i].used = TRUE;
                plf_laser_put_tickets[i].x = orig_x;
                plf_laser_put_tickets[i].y = orig_y;
                plf_laser_put_tickets[i].dir = dir;
                plf_laser_put_tickets_mark = curr;
                return TRUE;
            }
        }
        return FALSE;
    }

    PlfTile * tile = PLF_get_tile(orig_x, orig_y);
    const u16 OUTFLAG = PLF_LASER_OUT_R << dir;

    if(tile->laser & OUTFLAG)
        return FALSE; // laser coming out of here in this dir already
    else
        tile->laser |= OUTFLAG;

    u16 curr_x = orig_x;
    u16 curr_y = orig_y;
    bool terminated = FALSE;
    u16 counter = 0;

    while(!terminated)
    {
        u8 out_dir = dir; // assume laser omes out of same dir
        // update x and y of next in
        DIR_UPDATE_XY(curr_x, curr_y, dir);
        tile = PLF_get_tile_safe(curr_x, curr_y);
        if(!tile)
        {
            // finish if field ended
            break;
        }

        tile->laser |= (PLF_LASER_IN_R << dir);

        if(tile->pobj)
        {
            enum PobjDamageType damage = POBJ_DAMAGE_LASER;
            Pobj_event(Pobj_get_data(tile->pobj), POBJ_EVT_DAMAGE, &damage);
        }

        if(tile->attrs & PLF_ATTR_LASER_SOLID)
        {
            terminated = TRUE;
        }
        else if(tile->pobj)
        {
            enum PobjLaserBehavior behavior = POBJ_LASER_PASS;
            Pobj_event(Pobj_get_data(tile->pobj), POBJ_EVT_LASER_QUERY, &behavior);
            out_dir = _PLF_laser_behavior_apply(behavior, dir);
            if(out_dir == 0xff)
                terminated = TRUE;
        }

        if(!terminated)
            tile->laser |= (PLF_LASER_OUT_R << out_dir);

        _PLF_laser_gfx_update(curr_x, curr_y, FALSE);

        dir = out_dir;
        if(++counter == 0xfff)
        {
            // sentinel
            SFX_play(SFX_glass); // TODO remove
            break;
        }
    }

    _PLF_laser_gfx_update(orig_x, orig_y, FALSE);

    return TRUE;
}

void PLF_laser_block(u16 plf_x, u16 plf_y)
{
    PlfTile *orig_tile = PLF_get_tile_safe(plf_x, plf_y);
    if(!orig_tile || !orig_tile->laser)
        return;

    for(u8 orig_out = 0; orig_out < 4; orig_out++)
    {
        const u8 OUT_BIT = PLF_LASER_OUT_R << orig_out;

        if(orig_tile->laser & OUT_BIT)
        {
            // process this inital direction

            u8 in_dir = orig_out;
            u16 curr_x = plf_x;
            u16 curr_y = plf_y;
            u16 counter = 0;
            while(TRUE)
            {
                // update x and y of next in
                DIR_UPDATE_XY(curr_x, curr_y, in_dir);
                u8 out_dir = in_dir; // assume this laser will leave the other way

                PlfTile * tile = PLF_get_tile_safe(curr_x, curr_y);
                if(!tile)
                {
                    // finish if field ended
                    break;
                }

                if(!(tile->laser & (PLF_LASER_IN_R << in_dir)))
                {
                    // finish if laser stoppped for some reason
                    break;
                }

                tile->laser &= ~(PLF_LASER_IN_R << in_dir); // remove laser in bit


                if(tile->attrs & PLF_ATTR_LASER_SOLID)
                {
                    out_dir = 0xff;
                }
                else if(tile->pobj)
                {
                    enum PobjLaserBehavior behavior = POBJ_LASER_PASS;
                    Pobj_event(Pobj_get_data(tile->pobj), POBJ_EVT_LASER_QUERY, &behavior);
                    out_dir = _PLF_laser_behavior_apply(behavior, in_dir);
                }

                if(out_dir != 0xff)
                {
                    if(!(tile->laser & (PLF_LASER_OUT_R << out_dir)))
                    {
                        // laser desync (no out bit corresponds)
                        break;
                    }
                    // this laser not coming out of current tile anymore
                    // remove laser out bit
                    tile->laser &= ~(PLF_LASER_OUT_R << out_dir);
                    in_dir = out_dir; // in direction for next tile
                    _PLF_laser_gfx_update(curr_x, curr_y, FALSE);
                }
                else
                {
                    // laser was terminated (no out)
                    _PLF_laser_gfx_update(curr_x, curr_y, FALSE);
                    break;
                }

                if(++counter == 0xfff)
                {
                    // sentinel
                    SFX_play(SFX_glass); // TODO remove
                    break;
                }
            }
        }
    }

    // no lasers coming out of recalc point anymore
    orig_tile->laser &= 0xf;

    _PLF_laser_gfx_update(plf_x, plf_y, FALSE);
}


void PLF_laser_recalc(u16 plf_x, u16 plf_y)
{
    PlfTile *orig_tile = PLF_get_tile_safe(plf_x, plf_y);
    if(!orig_tile || !orig_tile->laser)
        return;

    PLF_laser_block(plf_x, plf_y);


    enum PobjLaserBehavior behavior = POBJ_LASER_PASS;
    if(orig_tile->pobj)
        Pobj_event(Pobj_get_data(orig_tile->pobj), POBJ_EVT_LASER_QUERY, &behavior);

    // now, for every laser coming in, figure out what happens
    if(behavior != POBJ_LASER_BLOCK && !(orig_tile->attrs & PLF_ATTR_LASER_SOLID))
    {
        for(int in_dir = 0; in_dir < 4; in_dir++)
        {
            if(!(orig_tile->laser & (PLF_LASER_IN_R << in_dir)))
                continue;

            u8 out_dir = _PLF_laser_behavior_apply(behavior, in_dir);

            if(DEBUG_LASER_BEHAVIOR)
            {
                char buf[40];
                sprintf(buf, "%d, %d, %d", behavior, in_dir, out_dir);
                VDP_drawTextBG(BG_A, buf, 0, 0);
            }

            if(out_dir != 0xff)
            {
                PLF_laser_put(plf_x, plf_y, out_dir); // TODO optimize: updating origin gfx many times
                // orig_tile->laser |= (PLF_LASER_OUT_R << out_dir); already set by above
            }
        }
    }

    _PLF_laser_gfx_update(plf_x, plf_y, FALSE);
}



void _PLF_laser_gfx_update(u16 x, u16 y, bool force_sprite)
{
    PlfTile * const t = &plf_tiles[x + plf_w * y];
    const u8 attrs = t->attrs;
    const u8 laser = t->laser;
    const bool plane_a_usable = plf_plane_a_alloc? !PLANE_A_ALLOCATION_AT(x, y) : TRUE;

    enum PlfLaserGfx final_gfx = PLF_LASER_GFX_NONE;

    if(laser)
    {
        enum PobjLaserBehavior behavior = POBJ_LASER_PASS;

        if(t->pobj)
            Pobj_event(Pobj_get_data(t->pobj), POBJ_EVT_LASER_QUERY, &behavior);

        if(laser == PLF_LASER_OUT_D)
        {
            // laser going out the bottom and nothing else (bottom cannon)
            force_sprite = TRUE;
            final_gfx = PLF_LASER_GFX_TERM_D;
        }
        else if (laser == PLF_LASER_IN_U)
        {
            // single termination
            final_gfx = PLF_LASER_GFX_TERM_D;
        }
        else if (laser == PLF_LASER_IN_R)
        {
            // single terminatiUn
            final_gfx = PLF_LASER_GFX_TERM_L;
        }
        else if (laser == PLF_LASER_IN_L)
        {
            // single termination
            final_gfx = PLF_LASER_GFX_TERM_R;
        }
        else if (laser == (PLF_LASER_IN_U|PLF_LASER_OUT_U) || laser == (PLF_LASER_IN_D|PLF_LASER_OUT_D) || laser == (PLF_LASER_IN_D|PLF_LASER_OUT_D|PLF_LASER_IN_U|PLF_LASER_OUT_U))
        {
            // pure vertical
            final_gfx = PLF_LASER_GFX_V;
        }
        else if (laser == (PLF_LASER_IN_L|PLF_LASER_OUT_L) || laser == (PLF_LASER_IN_R|PLF_LASER_OUT_R) || laser == (PLF_LASER_IN_R|PLF_LASER_OUT_R|PLF_LASER_IN_L|PLF_LASER_OUT_L))
        {
            // pure horizontal
            final_gfx = PLF_LASER_GFX_H;
        }
        else if (laser == (PLF_LASER_IN_U|PLF_LASER_IN_D))
        {
            // double termination
            final_gfx = PLF_LASER_GFX_TERM_D;
        }
        else if (laser == (PLF_LASER_IN_L|PLF_LASER_IN_R))
        {
            // double termination
            final_gfx = PLF_LASER_GFX_TERM_R|PLF_LASER_GFX_TERM_L;
        }else
        {
            // something more complex is going on

            enum PobjLaserBehavior behavior = (t->attrs&PLF_ATTR_LASER_SOLID)? POBJ_LASER_BLOCK : POBJ_LASER_PASS;
            if(t->pobj)
                Pobj_event(Pobj_get_data(t->pobj), POBJ_EVT_LASER_QUERY, &behavior);

            for(int in_dir = 0; in_dir < 4; in_dir++)
            {
                // laser in from this direction
                u8 in_bit = PLF_LASER_IN_R << in_dir;
                if(!(laser&in_bit))
                    continue;

                u8 out_dir = _PLF_laser_behavior_apply(behavior, in_dir);
                final_gfx |= _PLF_laser_in_out_to_gfx(in_dir, out_dir);
            }

        }

        // adjustments
        if(final_gfx == (PLF_LASER_GFX_H|PLF_LASER_GFX_V))
            final_gfx = PLF_LASER_GFX_HV;
    }


    bool plane_a_use = plane_a_usable && !(t->attrs & PLF_ATTR_COVERED_ANY);
    u8 plane_a_frame = PLF_LASER_FRAME_NONE;
    u8 front_sprite_tile = PLF_LASER_FRAME_NONE;
    u8 back_sprite_tile = PLF_LASER_FRAME_NONE;

    // now, distribute gfx bits into frames in the above slots
    while (final_gfx)
    {
        u16 curr_gfx = (final_gfx & ((u16)-((s16)final_gfx))); // get lsb
        bool always_back_sprite = (curr_gfx == PLF_LASER_GFX_QUAD_UL || curr_gfx == PLF_LASER_GFX_QUAD_UR);

        if(always_back_sprite && back_sprite_tile == PLF_LASER_FRAME_NONE)
        {
            back_sprite_tile = _PLF_laser_gfx_to_frame(curr_gfx);
        }
        else if(plane_a_frame == PLF_LASER_FRAME_NONE && plane_a_use && !force_sprite)
            plane_a_frame = _PLF_laser_gfx_to_frame(curr_gfx);
        else if(front_sprite_tile == PLF_LASER_FRAME_NONE)
        {
            front_sprite_tile = _PLF_laser_gfx_to_frame(curr_gfx);
        }
        else if(back_sprite_tile == PLF_LASER_FRAME_NONE)
        {
            back_sprite_tile = _PLF_laser_gfx_to_frame(curr_gfx);
        }
        else
            final_gfx = 0;  // TOO MUCH GFX for a single tile

        final_gfx &= ~curr_gfx;
    }

    // update plane a
    if(plane_a_frame != PLF_LASER_FRAME_NONE)
    {
        u16 final_frame_idx = PLF_theme_data_idx_table(PLF_THEME_LASER_LIGHT)[0][plane_a_frame];
        const u16 tile_attr = TILE_ATTR_FULL(PAL_LINE_BG_1, 1,
                                                0,
                                                0,
                                                final_frame_idx);
        PLF_plane_draw(FALSE, x, y, tile_attr);
    }
    else if(attrs & PLF_ATTR_PLANE_A_LASER)
    {
        // clear plane a
        PLF_plane_clear(FALSE, x, y);
    }

    // deal with sprites
    // first we go through the existing sprites
    // reuse them if we have sprites to place
    // if we don't have sprites to place but there are sprites there, free them
    PlfLaserSprEntry *entry = NULL;
    u8 start = 0;
    while((entry = _PLF_laser_spr_find_first(x, y, &start)))
    {
        if(front_sprite_tile == PLF_LASER_FRAME_NONE && back_sprite_tile == PLF_LASER_FRAME_NONE)
            _PLF_laser_spr_free(entry);
        else if(front_sprite_tile != PLF_LASER_FRAME_NONE)
        {
            _PLF_laser_spr_setup(entry, x, y, front_sprite_tile, FALSE); // takes care of reusing sprite if it already exists
            front_sprite_tile = PLF_LASER_FRAME_NONE;
        }
        else if (back_sprite_tile != PLF_LASER_FRAME_NONE)
        {
            _PLF_laser_spr_setup(entry, x, y, back_sprite_tile, TRUE); // same
            back_sprite_tile = PLF_LASER_FRAME_NONE;
        }
    }



    // if we still have sprites to place, allocate new ones
    if(front_sprite_tile != PLF_LASER_FRAME_NONE || back_sprite_tile != PLF_LASER_FRAME_NONE)
    {
        start = 0;
        while((entry = _PLF_laser_spr_find_free(&start)))
        {
            if(front_sprite_tile != PLF_LASER_FRAME_NONE)
            {
                _PLF_laser_spr_setup(entry, x, y, front_sprite_tile, FALSE);
                front_sprite_tile = PLF_LASER_FRAME_NONE;
            }
            else if (back_sprite_tile != PLF_LASER_FRAME_NONE)
            {
                _PLF_laser_spr_setup(entry, x, y, back_sprite_tile, TRUE);
                back_sprite_tile = PLF_LASER_FRAME_NONE;
            }
            else break;
        }
    }

    // if(front_sprite_tile || back_sprite_tile) {...}  no free sprite entries, don't place anything for now
}


inline enum PlfLaserGfx _PLF_laser_in_out_to_gfx(u8 in_dir, u8 out_dir)
{
    switch (in_dir)
    {
        case DIR_R:
            switch (out_dir)
            {
                case DIR_R:
                    return PLF_LASER_GFX_H;
                case DIR_L:
                case 0xff:
                    return PLF_LASER_GFX_TERM_L;
                case DIR_U:
                    return PLF_LASER_GFX_QUAD_UL;
                case DIR_D:
                    return PLF_LASER_GFX_QUAD_DL;
            }
            break;
        case DIR_L:
            switch (out_dir)
            {
                case DIR_L:
                    return PLF_LASER_GFX_H;
                case DIR_R:
                case 0xff:
                    return PLF_LASER_GFX_TERM_R;
                case DIR_U:
                    return PLF_LASER_GFX_QUAD_UR;
                case DIR_D:
                    return PLF_LASER_GFX_QUAD_DR;
            }
            break;
        case DIR_U:
            switch (out_dir)
            {
                case DIR_L:
                    return PLF_LASER_GFX_QUAD_DL;
                case DIR_R:
                    return PLF_LASER_GFX_QUAD_DR;
                case DIR_U:
                    return PLF_LASER_GFX_V;
                case 0xff:
                case DIR_D:
                    return PLF_LASER_GFX_TERM_D;
            }
            break;
        case DIR_D:
            switch (out_dir)
            {
                case DIR_L:
                    return PLF_LASER_GFX_QUAD_UL;
                case DIR_R:
                    return PLF_LASER_GFX_QUAD_UR;
                case DIR_D:
                    return PLF_LASER_GFX_V;
                case 0xff:
                case DIR_U:
                    return PLF_LASER_GFX_NONE;
            }
            break;

    };

    return PLF_LASER_GFX_NONE;
}


inline enum PlfLaserFrame _PLF_laser_gfx_to_frame(enum PlfLaserGfx gfx)
{
    switch(gfx)
    {
        case PLF_LASER_GFX_H:
            return PLF_LASER_FRAME_H;
        case PLF_LASER_GFX_V:
            return PLF_LASER_FRAME_V;
        case PLF_LASER_GFX_HV:
            return PLF_LASER_FRAME_CROSS;
        case PLF_LASER_GFX_QUAD_DL:
            return PLF_LASER_FRAME_DL;
        case PLF_LASER_GFX_QUAD_DR:
            return PLF_LASER_FRAME_DR;
        case PLF_LASER_GFX_QUAD_UL:
            return PLF_LASER_FRAME_UL;
        case PLF_LASER_GFX_QUAD_UR:
            return PLF_LASER_FRAME_UR;
        case PLF_LASER_GFX_TERM_D:
            return PLF_LASER_FRAME_DT;
        case PLF_LASER_GFX_TERM_R:
            return PLF_LASER_FRAME_RT;
        case PLF_LASER_GFX_TERM_L:
            return PLF_LASER_FRAME_LT;
        default:
            return PLF_LASER_FRAME_NONE;
    }
}


inline u8 _PLF_laser_behavior_apply(u8 behavior, u8 in_dir)
{
    if(behavior == POBJ_LASER_PASS)
        return in_dir;

    if(behavior == POBJ_LASER_REFLECT_RIGHT_DOWN)
    {
        switch(in_dir)
        {
            case DIR_L:
                return DIR_U;
                break;
            case DIR_R:
                return DIR_D;
                break;
            case DIR_U:
                return DIR_L;
                break;
            case DIR_D:
                return DIR_R;
                break;
        }
    }
    else if(behavior == POBJ_LASER_REFLECT_RIGHT_UP)
    {
        switch(in_dir)
        {
            case DIR_L:
                return DIR_D;
                break;
            case DIR_R:
                return DIR_U;
                break;
            case DIR_U:
                return DIR_R;
                break;
            case DIR_D:
                return DIR_L;
                break;
        }
    }

    return 0xff;
}


PlfLaserSprEntry *_PLF_laser_spr_find_free(u8 *start)
{
    for(u8 i = *start; i < PLF_LASER_SPRITE_MAX; i++)
    {
        if(!plf_laser_sprites[i].spr)
        {
            (*start) = (i+1)%PLF_LASER_SPRITE_MAX;
            return plf_laser_sprites + i;
        }
    }
    return NULL;
}

PlfLaserSprEntry *_PLF_laser_spr_find_first(u8 x, u8 y, u8 *start)
{
    for(u8 i = *start; i < PLF_LASER_SPRITE_MAX; i++)
    {
        if(plf_laser_sprites[i].spr && plf_laser_sprites[i].px == x && plf_laser_sprites[i].py == y)
        {
            (*start) = (i+1)%PLF_LASER_SPRITE_MAX;
            return plf_laser_sprites + i;
        }
    }
    return NULL;
}


bool _PLF_laser_spr_setup(PlfLaserSprEntry * entry, u8 x, u8 y, enum PlfLaserFrame frame, bool behind)
{
    if(!entry->spr)
        entry->spr = SPR_addSprite(plf_theme_sprite_defs[PLF_THEME_LASER_LIGHT],
                                   x*16, y*16,
                                   TILE_ATTR(plf_theme_palette_line[PLF_THEME_LASER_LIGHT],0,0,0));
    if(entry->spr)
    {
        SPR_setAutoTileUpload(entry->spr, FALSE);
        SPR_setVRAMTileIndex(entry->spr, PLF_theme_data_idx_table(PLF_THEME_LASER_LIGHT)[0][frame]);
        SPR_setDepth(entry->spr, PLF_get_sprite_depth(FIX16(x), FIX16(y)) + (behind? 1 : -1));
        entry->px = x;
        entry->py = y;
        return TRUE;
    }

    // could not create sprite
    entry->px = entry->py = -1;
    return FALSE;
}

void _PLF_laser_spr_free(PlfLaserSprEntry * entry)
{
    if(entry->spr)
    {
        SPR_releaseSprite(entry->spr);
        entry->spr = NULL;
    }
    entry->px = entry->py = -1;
}

void PLF_cover(u16 plf_x, u16 plf_y, bool player)
{
    PlfTile * const tile = PLF_get_tile_safe(plf_x, plf_y);
    if(!tile)
        return;

    const enum PlfAttrBits bit = player? PLF_ATTR_PLANE_A_PLAYER : PLF_ATTR_PLANE_A_KEEPOUT;
    u8 prev = tile->attrs & bit;
    tile->attrs |= bit;
    if(prev != (tile->attrs & bit))
        _PLF_laser_gfx_update(plf_x, plf_y, FALSE);
}

void PLF_uncover(u16 plf_x, u16 plf_y, bool player)
{
    PlfTile * const tile = PLF_get_tile_safe(plf_x, plf_y);
    if(!tile)
        return;

    const enum PlfAttrBits bit = player? PLF_ATTR_PLANE_A_PLAYER : PLF_ATTR_PLANE_A_KEEPOUT;
    u8 prev = tile->attrs & bit;
    tile->attrs &= ~bit;
    if(prev != (tile->attrs & bit))
        _PLF_laser_gfx_update(plf_x, plf_y, FALSE);
}

void PLF_plane_draw(bool planeB, u16 x, u16 y, u16 tile_attr)
{
    PlfTile * const tile = PLF_get_tile_safe(x, y);
    if(!tile)
        return;

    GFX_draw_sprite_in_plane_2x2_inline(planeB? BG_B : BG_A, x*2, y*2, tile_attr);

    tile->attrs |= (planeB? PLF_ATTR_PLANE_B_OBJ : PLF_ATTR_PLANE_A_LASER);
}


void PLF_plane_clear(bool planeB, u16 x, u16 y)
{
    PlfTile * const tile = PLF_get_tile_safe(x, y);
    if(!tile)
        return;

    if(planeB)
    {
        u16 orig_tiles[4];
        MAP_getTilemapRect(m_b, x, y, 1, 1, FALSE, orig_tiles);
        VDP_setTileMapXY(BG_B, orig_tiles[0], x*2, y*2);
        VDP_setTileMapXY(BG_B, orig_tiles[1], x*2+1, y*2);
        VDP_setTileMapXY(BG_B, orig_tiles[2], x*2, y*2+1);
        VDP_setTileMapXY(BG_B, orig_tiles[3], x*2+1, y*2+1);
        tile->attrs &= ~PLF_ATTR_PLANE_B_OBJ;
    }
    else
    {
        VDP_clearTileMapRect(BG_A, x*2, y*2, 2, 2);
        tile->attrs &= ~PLF_ATTR_PLANE_A_LASER;
    }
}


bool PLF_plane_a_map_usage(u16 x, u16 y)
{
    return PLANE_A_ALLOCATION_AT(x, y);
}


bool PLF_plane_a_avail_obj(u16 x, u16 y)
{
    PlfTile * const tile = PLF_get_tile_safe(x, y);
    if(!tile)
        return FALSE;

    if(PLANE_A_ALLOCATION_AT(x, y))
        return FALSE;

    if(tile->attrs & PLF_ATTR_COVERED_ANY)
        return FALSE;
}


bool PLF_plane_a_cover(u16 x, u16 y, bool player);


bool PLF_plane_a_uncover(u16 x, u16 y, bool player);

void PLF_plane_a_refresh()
{
    // redraw the whole of plane A
    s16 plf_scroll_x = fix16ToRoundedInt(fix16Sub(plf_cam_cx, FIX16(160)));
    s16 plf_scroll_y = fix16ToRoundedInt(fix16Sub(plf_cam_cy, FIX16(96)));
    MAP_scrollToEx(m_a, plf_scroll_x, plf_scroll_y, TRUE);
    DMA_flushQueue();

    for(int x = 0; x < plf_w; x++)
    {
        for(int y = 0; y < plf_h; y++)
        {
            PlfTile * const tile = PLF_get_tile(x, y);
            if(tile->attrs & PLF_ATTR_PLANE_A_LASER)
            {
                // TODO can objects also reuse plane a? for now only laser
                _PLF_laser_gfx_update(x, y, FALSE);
            }
        }
    }
    DMA_flushQueue();
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


void PLF_update_objects(u32 framecounter)
{
    Pobj_event_to_all(POBJ_EVT_FRAME, &framecounter, TRUE);
}

const SpriteDefinition* PLF_theme_data_sprite_def(enum PlfThemeData ref)
{
    return plf_theme_sprite_defs[ref];
}

u16** PLF_theme_data_idx_table(enum PlfThemeData ref)
{
    return plf_theme_tile_indices[ref];
}

u8 PLF_theme_data_pal_line(enum PlfThemeData ref)
{
    return plf_theme_palette_line[ref];
}

