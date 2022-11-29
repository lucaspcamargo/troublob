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



void PLF_init();

void PLF_cam_to(fix16 cx, fix16 cy);

PlfTile PLF_get_tile(u16 pf_x, u16 pf_y);

bool PLF_player_pathfind(u16 px, u16 py, u16 destx, u16 desty);

bool PLF_player_path_next(u16 px, u16 py, u16 *nextx, u16 *nexty);

/*
 * Note: forceRedraw will call vblank processing twice for uploading map data to planes
 * */
void PLF_update_scroll(bool forceRedraw);
