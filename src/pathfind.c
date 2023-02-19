#include "pathfind.h"


PathfindingNode PATH_nodes[PATHFIND_MAX] __attribute__ ((aligned (4)));
PATH_IDX_TYPE PATH_nodes_len = 0;
u8 PATH_bitmap[PATH_BITMAP_LEN];

#define PATH_POS_IS_WALL(x, y, attrs, stride_x, stride_y, wall_bit) ( *((attrs)+((stride_x)*(x)+(stride_y)*(y))) & (wall_bit) )


bool PATH_find( const u8 pfw, const u8 pfh, // playfield dims
                const u8 sx, const u8 sy, // start position
                u8 ex, const u8 ey, // goal position
                const u8 * const attrs, // playfield attribute addr
                const u16 stride_x, const u16 stride_y, // how to address playfield attributes (row-major or col-major)
                const u8 wall_bit // bits that mean position is blocked
                )
{
    PATH_nodes_len = 0; // globally, the node list is now empty

    if(sx==ex && sy==ey)
        return PATH_ALREADY_THERE;
    // init first path node (goal)
    PATH_nodes[0].x = ex;
    PATH_nodes[0].y = ey;
    PATH_nodes[0].dist = 0;

    // setup vars
    BITMAP_CLEAR(PATH_bitmap);
    BITMAP_SET(PATH_bitmap, ex, ey);
    PATH_IDX_TYPE curr_len = 1;
    PATH_IDX_TYPE next_len = curr_len;
    u16 ctr = 0, expand_start = 0;
    bool found_start = FALSE;

    while((ctr<PATHFIND_MAX) && !found_start)
    {
        // for every node on the list (yet to be expanded)
        for(u8 idx = expand_start; idx < curr_len; idx++)
        {
            const PATH_DIST_TYPE next_dist_ortho = PATH_nodes[idx].dist + 1;
            // PATH_DIST_TYPE next_dist_diagonal = PATH_nodes[idx].dist + 3; -- TODO try later (with manhattan distance [+1]?)

            //check all four directions, maybe add to list
            for(u8 dir = 0; dir < 4; dir++)
            {
                u8 tx = PATH_nodes[idx].x + (dir<2?(dir%2?0xff:0x01):0);
                u8 ty = PATH_nodes[idx].y + (dir<2?0:(dir%2?0xff:0x01));
                if(tx >= pfw || ty >= pfh)
                    continue; // out of bounds

                if(BITMAP_IS_SET(PATH_bitmap, tx, ty))
                    continue; // we already have a node at this position

                if(PATH_POS_IS_WALL(tx, ty, attrs, stride_x, stride_y, wall_bit))
                {
                    if(DEBUG_PATHFINDING && DEBUG_PATHFINDING_FIELD)
                    {
                        VDP_drawTextBG(BG_A, "XX", tx*2, ty*2);
                    }
                    BITMAP_SET(PATH_bitmap, tx, ty); // dont even test for this wall again
                    continue; // wall
                }

                //if we got here, add a new node to the list
                BITMAP_SET(PATH_bitmap, tx, ty);
                PATH_nodes[next_len].x = tx;
                PATH_nodes[next_len].y = ty;
                PATH_nodes[next_len].dist = next_dist_ortho;
                next_len++;
                if(next_len == (PATHFIND_MAX+1))
                    return PATH_LIST_OVERFLOW;

                if(DEBUG_PATHFINDING && DEBUG_PATHFINDING_FIELD)
                {
                    char buf[3];
                    intToHex(next_dist_ortho, buf, 2);
                    VDP_drawTextBG(BG_A, buf, tx*2, ty*2);
                }

                if(tx==sx && ty==sy)
                {
                    if(DEBUG_PATHFINDING && DEBUG_PATHFINDING_FIELD)
                        VDP_drawTextBG(BG_A, "SS", tx*2, ty*2);
                    found_start = TRUE;
                }
            }
        }
        if(DEBUG_PATHFINDING && DEBUG_PATHFINDING_FIELD)
            for(int j = 0; j < (1+DEBUG_PATHFINDING_SLOW); j++)
                SYS_doVBlankProcess(); // try to render every step on screen

        if(next_len == curr_len)
            return PATH_NOT_FOUND; // if list didn't grow at all, we must be stuck

        expand_start = curr_len; // start expanding from next newly expanded node
        curr_len = next_len;
        ctr++;
    }
    if (!found_start)
        return PATH_EXCEEDED_MAX_ITERS;
    // we found the start_idx
    // now from the start, we navigate the map, always choosing the next node with the smallest distance

    PATH_nodes_len = curr_len;
    return PATH_FOUND;
};


PathfindingNode* PATH_node_at(u8 x, u8 y)
{
    for(PATH_IDX_TYPE i = 0; i < PATH_nodes_len; i++)
    {
        if(PATH_nodes[i].x == x && PATH_nodes[i].y == y)
            return PATH_nodes + i;
    }
    return NULL;
}

bool PATH_next( const u8 pfw, const u8 pfh,
                u8 currx, u8 curry,
                u8 *nextx, u8 *nexty)
{
    PATH_DIST_TYPE found_dist = 0xff;
    u8 found_x = 0;
    u8 found_y = 0;
    const u8 min_x = currx? currx-1 : currx;
    const u8 max_x = currx<(pfw-1)? currx+1 : currx;
    const u8 min_y = curry? curry-1 : curry;  // delicious curry
    const u8 max_y = curry<(pfh-1)? curry+1 : curry;
    for(PATH_IDX_TYPE i = 0; i < PATH_nodes_len; i++)
    {
        if(PATH_nodes[i].x == currx && PATH_nodes[i].y == curry)
            continue; // skip origin node
        if((min_x <= PATH_nodes[i].x) &&
           (PATH_nodes[i].x <= max_x) &&
           (min_y <= PATH_nodes[i].y) &&
           (PATH_nodes[i].y <= max_y))
        {
            // within movement range
            if(PATH_nodes[i].dist < found_dist)
            {
                // select this one
                found_x = PATH_nodes[i].x;
                found_y = PATH_nodes[i].y;
                found_dist = PATH_nodes[i].dist;
            }
        }
    }

    *nextx = found_x;
    *nexty = found_y;
    return found_dist != 0xff && (found_x != currx || found_y != curry);
}

u16 PATH_curr_node_count()
{
    return PATH_nodes_len;
}
