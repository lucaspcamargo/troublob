#pragma once

#include <genesis.h>

#include "dweep_config.h"
#include "resources.h"
#include "pathfind.h"
#include "tools.h"

// TYPE DFINITIONS

// uniform attributes for every playfield tile
enum PlfAttrBits {
    PLF_ATTR_PLAYER_SOLID = 1,    // cannot walk or put objects
    PLF_ATTR_PLANE_A_REUSED = 2,  // in this position, plane A is being used by a laser
    PLF_ATTR_PLANE_B_REUSED = 4,  // in this position, plane B is being used by object
    PLF_ATTR_DANGER = 8,          // dweep is afraid of this tile (laser handled separately)
    PLF_ATTR_LASER_SOLID = 16,    // laser stops here
    PLF_ATTR_HOLE = 32,           // player or objects cannot be placed
    PLF_ATTR_COVERED_OBJ = 64,        // an object sprite overlaps this tile, don't use plane A
    PLF_ATTR_COVERED_PLAYER = 128,        // a player sprite overlaps this tile, don't use plane A
} ENUM_PACK;

static const u8 PLF_ATTR_COVERED_ANY = (PLF_ATTR_COVERED_OBJ|PLF_ATTR_COVERED_PLAYER);

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
} ENUM_PACK;

#define PLF_LASER_IN_ALL (PLF_LASER_IN_R|PLF_LASER_IN_L|PLF_LASER_IN_U|PLF_LASER_IN_D)
#define PLF_LASER_OUT_ALL (PLF_LASER_OUT_R|PLF_LASER_OUT_L|PLF_LASER_OUT_U|PLF_LASER_OUT_D)


// Do not confuse with Tiled internal "type" attribute, only used for type_filter OBJECTS functionality
// I could add flip/rotation flags for directional objects, BUT i'll just list them for every direction,
//     since most objects are not directional
// This is also different to POBJ_TYPE, although it may seem redundant
// PlfObjType are the object types that can come from level data, no subtypes
// PobjType are the types of objects that exist at runtime, they have subtypes
enum PlfObjectType {
    PLF_OBJ_WALL    = 0, // usually rect, marks any tile that contains it as a wall
    PLF_OBJ_PLAYER  = 1, // usually point, where dweep begins
    PLF_OBJ_GOAL    = 2, // end goal of level
    PLF_OBJ_LASER_R = 3, // laser, right
    PLF_OBJ_LASER_L = 4, // laser, left
    PLF_OBJ_LASER_U = 5, // laser, up
    PLF_OBJ_LASER_D = 6, // laser, down
    PLF_OBJ_MIRROR  = 7, // laser, reflects left up /.
    PLF_OBJ_MIRROR2 = 8, // mirror, reflects left down \.
    PLF_OBJ_HEAT    = 9, // heat plate
    PLF_OBJ_COLD   = 10, // ice plate
    PLF_OBJ_BOMB   = 11, // bomb

    PLF_OBJ_HOLE = 127,  // like wall, but a hole
    PLF_OBJ_ITEM_BASE = 128,    // initial tool index
    PLF_OBJ_MAX =  PLF_OBJ_ITEM_BASE + TOOL_COUNT,         // number of existing specs, value >= to this is invalid, ignored
} ENUM_PACK;

#define PLF_OBJ_IS_Y_BOTTOM(spec_enum) ((spec_enum) && (spec_enum) != PLF_OBJ_HOLE) // specs that have y origin in bottom of box (tile/obj)


enum PlfThemeData{
    PLF_THEME_LASER_LIGHT,
    PLF_THEME_LASER_CANNON,
    PLF_THEME_MIRROR,
    PLF_THEME_TOOLS,
    PLF_THEME_GOAL,
    PLF_THEME_GOAL_BABIES,
    PLF_THEME_BOMB,
    PLF_THEME_HOT,
    PLF_THEME_COLD,
    PLF_THEME_EXPLOSION,
    PLF_THEME_COUNT
} ENUM_PACK;

typedef struct PlfTile_st {
    u8 attrs;
    u8 laser;
    void* pobj;
} PlfTile;



void PLF_init(u16 lvl_id);
void PLF_reset(u16 lvl_id);
void PLF_destroy();

void PLF_cam_to(fix16 cx, fix16 cy);

PlfTile * PLF_get_tile(u16 pf_x, u16 pf_y);
PlfTile * PLF_get_tile_safe(u16 pf_x, u16 pf_y);  // this one return NULL if tile out of bounds

void PLF_player_get_initial_pos(fix16 *dest_x, fix16 *dest_y);
bool PLF_player_pathfind(u16 px, u16 py, u16 destx, u16 desty);
bool PLF_player_path_next(u16 px, u16 py, u16 *nextx, u16 *nexty);

void* PLF_obj_create(u16 pobj_type, u16 pobj_subtype, u16 px, u16 py);   // returns PobjHnd
void* PLF_obj_at(u16 px, u16 py);       // returns PobjHnd
void  PLF_obj_destroy(u16 px, u16 py, void *evt_arg);   // destroy object at
void  PLF_obj_damage(u8 type, u16 px, u16 py);

bool PLF_laser_put(u16 orig_x, u16 orig_y, u8 dir);
void PLF_laser_block(u16 plf_x, u16 plf_y);  // undoes lasers that go out from the tile
void PLF_laser_recalc(u16 plf_x, u16 plf_y);  // undoes lasers that goes out from the tile (block), then puts them again

inline s16 PLF_get_sprite_depth(fix16 x, fix16 y) { (void) x; return 0x8000 - (y<<2); } // multiply y by 4 to give headroom for sprite ordering within tile
void PLF_cover(u16 plf_x, u16 plf_y, bool player);
void PLF_uncover(u16 plf_x, u16 plf_y, bool player);

/*
 * Note: forceRedraw will call vblank processing twice for uploading map data to planes
 * */
void PLF_update_scroll(bool forceRedraw);
void PLF_update_objects(u32 framecounter);

// reuse plane A (for laser) or plane B (for objects) to display 4 column-major tiles at position
// column major because we can use this with unoptimized sprite data
// tiles are marked as such via PLF_ATTR_PLANE_X_REUSED
void PLF_plane_draw(bool planeB, u16 x, u16 y, u16 tile_attr);

// undo a PLF_plane_draw at the specified position
void PLF_plane_clear(bool planeB, u16 x, u16 y);

// tell if

// plane a was garbled (usually by in-game menu)
// all in it needs to be redrawn
void PLF_plane_a_refresh();

// get a tile index table (u16**) preloaded by the playfield theming
// may return a NULL value in case gfx is not part of theme
const SpriteDefinition* PLF_theme_data_sprite_def(enum PlfThemeData ref);
u16** PLF_theme_data_idx_table(enum PlfThemeData ref);
u8 PLF_theme_data_pal_line(enum PlfThemeData ref); // same for palette
