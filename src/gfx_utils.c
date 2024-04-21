/*
This file is part of Dweep Genesis.

Dweep Genesis is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

Dweep Genesis is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with Foobar. If not, see <https://www.gnu.org/licenses/>.
*/

#include "gfx_utils.h"


#define GFX_QUEUE_MAX 128


enum GFXQueueOp
{
    GFX_OP_SPRITE_PLANE_2x2,
    GFX_OP_CLEAR_2x2
} ENUM_PACK;

typedef struct GFXQueueEntry_st
{
    enum GFXQueueOp op;
    u8 plane;
    u16 tile_x;
    u16 tile_y;
    u16 tile_attr;
} GFXQueueEntry;


static bool gfx_queue_on = FALSE;
static GFXQueueEntry gfx_queue[GFX_QUEUE_MAX];
static u8 gfx_queue_sz;


// private
void _GFX_vsync_cb();


void GFX_init_queue()
{
    if(gfx_queue_on)
        return;
    SYS_setVBlankCallback(&_GFX_vsync_cb);
    gfx_queue_on = TRUE;
    gfx_queue_sz = 0;
}

void GFX_stop_queue()
{
    if(!gfx_queue_on)
        return;
    SYS_setVBlankCallback(NULL);
    _GFX_vsync_cb(); // finish
    gfx_queue_on = FALSE;
}

bool GFX_is_queued()
{
    return gfx_queue_on;
}

void GFX_queue_clear()
{
    gfx_queue_sz = 0;
}


void GFX_draw_sprite_in_plane_2x2_immediate(VDPPlane plane, u16 x, u16 y, u16 tile_attr)
{
    if(tile_attr & TILE_ATTR_VFLIP_MASK)
    {
        if(tile_attr & TILE_ATTR_HFLIP_MASK)
        {
            // both flip
            VDP_setTileMapXY(plane, tile_attr  , x+1, y+1);
            VDP_setTileMapXY(plane, tile_attr+1, x+1, y);
            VDP_setTileMapXY(plane, tile_attr+2, x,   y+1);
            VDP_setTileMapXY(plane, tile_attr+3, x,   y);

        }
        else
        {
            // vflip
            VDP_setTileMapXY(plane, tile_attr  , x,   y+1);
            VDP_setTileMapXY(plane, tile_attr+1, x,   y);
            VDP_setTileMapXY(plane, tile_attr+2, x+1, y+1);
            VDP_setTileMapXY(plane, tile_attr+3, x+1, y);
        }
    }
    else
    {
        if(tile_attr & TILE_ATTR_HFLIP_MASK)
        {
            // hflip
            VDP_setTileMapXY(plane, tile_attr  , x+1, y);
            VDP_setTileMapXY(plane, tile_attr+1, x+1, y+1);
            VDP_setTileMapXY(plane, tile_attr+2, x,   y);
            VDP_setTileMapXY(plane, tile_attr+3, x,   y+1);
        }
        else
        {
            // no flip
            VDP_setTileMapXY(plane, tile_attr  , x,   y);
            VDP_setTileMapXY(plane, tile_attr+1, x,   y+1);
            VDP_setTileMapXY(plane, tile_attr+2, x+1, y);
            VDP_setTileMapXY(plane, tile_attr+3, x+1, y+1);
        }
    }
}


void GFX_draw_sprite_in_plane_2x2(VDPPlane plane, u16 x, u16 y, u16 tile_attr)
{
    if(gfx_queue_on)
    {
        gfx_queue[gfx_queue_sz].op = GFX_OP_SPRITE_PLANE_2x2;
        gfx_queue[gfx_queue_sz].plane = plane;
        gfx_queue[gfx_queue_sz].tile_x = x;
        gfx_queue[gfx_queue_sz].tile_y = y;
        gfx_queue[gfx_queue_sz].tile_attr = tile_attr;
        gfx_queue_sz++;
    }
    else
        GFX_draw_sprite_in_plane_2x2_immediate(plane, x, y, tile_attr);
}


void GFX_clear_2x2_immediate(VDPPlane plane, u16 x, u16 y)
{
    VDP_setTileMapXY(plane, 0, x,   y);
    VDP_setTileMapXY(plane, 0, x,   y+1);
    VDP_setTileMapXY(plane, 0, x+1, y);
    VDP_setTileMapXY(plane, 0, x+1, y+1);
}

void GFX_clear_2x2(VDPPlane plane, u16 x, u16 y)
{
    if(gfx_queue_on)
    {
        gfx_queue[gfx_queue_sz].op = GFX_OP_CLEAR_2x2;
        gfx_queue[gfx_queue_sz].plane = plane;
        gfx_queue[gfx_queue_sz].tile_x = x;
        gfx_queue[gfx_queue_sz].tile_y = y;
        gfx_queue[gfx_queue_sz].tile_attr = 0;
        gfx_queue_sz++;
    }
    else
        GFX_clear_2x2_immediate(plane, x, y);
}


void _GFX_vsync_cb()
{
    for(u8 i = 0; i < gfx_queue_sz; i++)
    {
        const GFXQueueEntry entry = gfx_queue[i];
        switch(entry.op)
        {
            case GFX_OP_SPRITE_PLANE_2x2:
                GFX_draw_sprite_in_plane_2x2_immediate(entry.plane, entry.tile_x, entry.tile_y, entry.tile_attr);
                break;
            case GFX_OP_CLEAR_2x2:
                GFX_clear_2x2_immediate(entry.plane, entry.tile_x, entry.tile_y);
                break;
        }
    }
    gfx_queue_sz = 0;
}
