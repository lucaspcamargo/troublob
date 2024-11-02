#pragma once

/*
This file is part of Blob Genesis.

Blob Genesis is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

Blob Genesis is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with Foobar. If not, see <https://www.gnu.org/licenses/>.
*/

#include <genesis.h>
#include "game_config.h"

#define PATHFIND_MAX PLAYFIELD_VIEW_W*PLAYFIELD_VIEW_H // 240, in this case, we can index nodes with u8, phew
#if PATHFIND_MAX > 254
#define PATH_IDX_TYPE u16
#else
#define PATH_IDX_TYPE u8
#endif
#define PATH_DIST_TYPE PATH_IDX_TYPE

// in the discovery stage, we use a bitmap to mark
// which tiles have already been navigated from (or are a wall?)
#define PATH_BITMAP_H_LEN (PLAYFIELD_VIEW_W/8+(PLAYFIELD_VIEW_W%8?1:0))
#define PATH_BITMAP_LEN (PLAYFIELD_VIEW_H*PATH_BITMAP_H_LEN)
#define BITMAP_IS_SET(bitmap, x, y) ((bitmap)[((x)/8)+((y)*PATH_BITMAP_H_LEN)]&(1<<(x&0x07)))
#define BITMAP_SET(bitmap, x, y) (bitmap)[((x)/8)+((y)*PATH_BITMAP_H_LEN)] |= (1<<(x&0x07))
#define BITMAP_UNSET(bitmap, x, y) (bitmap)[((x)/8)+((y)*PATH_BITMAP_H_LEN)] &= ~(1<<(x&0x07))
#define BITMAP_CLEAR(bitmap) memset((bitmap), 0, PATH_BITMAP_LEN)

// use a list of positions and distances as such
typedef struct
{
    u8 x, y;
    PATH_DIST_TYPE dist;
    //PATH_IDX_TYPE next; --> could be a linked list, not needed
    u8 pad; // BUT we need to align this struct to 4 bytes for speed
} PathfindingNode;

enum PathfindingResult {
    PATH_FOUND,
    PATH_NOT_FOUND,
    PATH_EXCEEDED_MAX_ITERS,
    PATH_LIST_OVERFLOW,
    PATH_ALREADY_THERE,
    PATH_UNKNOWN_ERR
} ENUM_PACK;


bool PATH_find(const u8 pfw, const u8 pfh, const u8 sx, const u8 sy, u8 ex, const u8 ey, const u8*const attrs, const u16 stride_x, const u16 stride_y, const u16 wall_bits);


PathfindingNode* PATH_node_at(u8 x, u8 y);

bool PATH_next( const u8 pfw, const u8 pfh,
                u8 currx, u8 curry,
                u8 *nextx, u8 *nexty);

u16 PATH_curr_node_count();
