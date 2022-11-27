#pragma once
#include "dweep_config.h"

#define PATHFIND_MAX PLAYFIELD_STD_W*PLAYFIELD_STD_H // 240, in this case, we can index nodes with u8, phew
#if PATHFIND_MAX > 255
#define PATH_IDX_TYPE u16
#else
#define PATH_IDX_TYPE u8
#endif
#define PATH_DIST_TYPE PATH_IDX_TYPE

// in the discovery stage, we use a bitmap to mark
// which tiles have already been navigated from (or are a wall?)
#define PATH_BITMAP_H_LEN (PLAYFIELD_STD_W/8+(PLAYFIELD_STD_W%8?1:0))
#define PATH_BITMAP_LEN (PLAYFIELD_STD_H*PATH_BITMAP_H_LEN)
#define BITMAP_IS_SET(bitmap, x, y) ((bitmap)[((x)/8)+((y)*PATH_BITMAP_H_LEN)]&(1<<(x&0x07)))
#define BITMAP_SET(bitmap, x, y) (bitmap)[((x)/8)+((y)*PATH_BITMAP_H_LEN)] |= (1<<(x&0x07))
#define BITMAP_UNSET(bitmap, x, y) (bitmap)[((x)/8)+((y)*PATH_BITMAP_H_LEN)] |= (1<<(x&0x07))
#define BITMAP_CLEAR(bitmap) memset((bitmap), 0, PATH_BITMAP_LEN)
u8 PATH_bitmap[PATH_BITMAP_LEN];

// use a list of positions and distances as such
typedef struct
{
    u8 x, y;
    PATH_DIST_TYPE dist;
    //PATH_IDX_TYPE next; --> could be a linked list, not needed
    u8 pad; // BUT we need to align this struct to 4 bytes for speed
} PathfindingNode;
PathfindingNode PATH_nodes[PATHFIND_MAX] __attribute__ ((aligned (4)));


enum PathfindingResult {
    PATH_FOUND,
    PATH_NOT_FOUND,
    PATH_EXCEEDED_MAX_ITERS,
    PATH_LIST_OVERFLOW,
    PATH_UNKNOWN_ERR
};

#define PATH_POS_IS_WALL(x, y, attrs, stride_x, stride_y, wall_bit) ( *((attrs)+((stride_x)*(x)+(stride_y)*(y))) & (wall_bit) )

// please compiler, inline this and cross out some constants
inline bool PATH_find(const u8 w, const u8 h, // playfield dims
                      const u8 sx, const u8 sy, // start position
                      u8 ex, const u8 ey, // goal position
                      const u8 * const attrs, // playfield attribute addr
                      const u8 stride_x, const u8 stride_y, // how to address playfield attributes (row-major or col-major)
                      const u8 wall_bit // bits that mean position is blocked
                      )
{
    const PathfindingNode initial = {ex, ey, 0, 0xff};
    PATH_nodes[0] = initial; // no need to clear mrest of list buffer
    BITMAP_CLEAR(PATH_bitmap);
    PATH_IDX_TYPE curr_len = 1;
    PATH_IDX_TYPE ctr = 0;
    bool found_start = FALSE;
    PATH_IDX_TYPE start_idx = -1;
    while(ctr < PATHFIND_MAX && !found_start)
    {
        PATH_IDX_TYPE next_len = curr_len; // buffer update to curr_len, do only after iterating the list
        // for every node on the list...
        for(u8 idx = 0; idx < curr_len; idx++)
        {
            const u8 curr_idx_x = PATH_nodes[idx].x;
            const u8 curr_idx_y = PATH_nodes[idx].y;
            if(BITMAP_IS_SET(PATH_bitmap, curr_idx_x, curr_idx_y))
                continue; // this position already visited
            else
                BITMAP_SET(PATH_bitmap, curr_idx_x, curr_idx_y); // well, visiting it now, mark it

            const PATH_DIST_TYPE next_dist_ortho = PATH_nodes[idx].dist + 1;
            // PATH_DIST_TYPE next_dist_diagonal = PATH_nodes[idx].dist + 3; -- TODO try later (with manhattan distance [+1])

            //check all four directions, maybe add to list
            for(u8 dir = 0; dir < 4; dir++)
            {
                u8 tx = PATH_nodes[idx].x + (dir<2?(dir%2?0xff:0x01):0);
                u8 ty = PATH_nodes[idx].y + (dir<2?0:(dir%2?0xff:0x01));
                if(tx >= w || ty >= h)
                    continue; // out of bounds

                if(BITMAP_IS_SET(PATH_bitmap, tx, ty))
                    continue; // test position already visited, no touchie

                if(PATH_POS_IS_WALL(tx, ty, attrs, stride_x, stride_y, wall_bit))
                {
                    if(DEBUG_PATHFINDING)
                        VDP_drawTextBG(BG_A, "XX", tx*2, ty*2);
                    continue; // wall
                }

                //if we got here, add this item to the list
                PATH_nodes[next_len].x = tx;
                PATH_nodes[next_len].y = ty;
                PATH_nodes[next_len].dist = next_dist_ortho;
                next_len++;

                if(DEBUG_PATHFINDING)
                {
                    char buf[] = "  ";
                    intToHex(next_dist_ortho, buf, 1);
                    VDP_drawTextBG(BG_A, buf, tx*2, ty*2);
                }

                if(tx==sx && ty==sy)
                {
                    if(DEBUG_PATHFINDING)
                        VDP_drawTextBG(BG_A, "S", tx*2, ty*2);
                    found_start = TRUE;
                    start_idx = next_len - 1;
                }
            }
        }
        if(DEBUG_PATHFINDING)
            for(int j = 0; j < (1+DEBUG_PATHFINDING_SLOW); j++)
                SYS_doVBlankProcess(); // try to render every step on screen

        if(next_len == curr_len)
            return PATH_NOT_FOUND; // if list didn't grow at all, we must be stuck

        curr_len = next_len;
        ctr++;
    }
    if (!found_start)
        return PATH_EXCEEDED_MAX_ITERS;
    // we found the start_idx
    // now from the start, we navigate the map, always choosing the next node with the smallest distance
    (void) start_idx; // suppress unused warning TODO use this?
    return PATH_FOUND;
};
