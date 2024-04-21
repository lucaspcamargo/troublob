#pragma once

#include <genesis.h>


inline void GFX_draw_sprite_in_plane_2x2_inline(VDPPlane plane, u16 x, u16 y, u16 tile_attr)
{
    if(tile_attr & TILE_ATTR_VFLIP_MASK)
    {
        if(tile_attr & TILE_ATTR_HFLIP_MASK)
        {
            // both flip
            VDP_setTileMapXY(plane, tile_attr  , x+1, y+1);
            VDP_setTileMapXY(plane, tile_attr+1, x+1, y);
            VDP_setTileMapXY(plane, tile_attr+2, x, y+1);
            VDP_setTileMapXY(plane, tile_attr+3, x, y);

        }
        else
        {
            // vflip
            VDP_setTileMapXY(plane, tile_attr  , x, y+1);
            VDP_setTileMapXY(plane, tile_attr+1, x, y);
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
            VDP_setTileMapXY(plane, tile_attr+2, x, y);
            VDP_setTileMapXY(plane, tile_attr+3, x, y+1);
        }
        else
        {
            // no flip
            VDP_setTileMapXY(plane, tile_attr  , x, y);
            VDP_setTileMapXY(plane, tile_attr+1, x, y+1);
            VDP_setTileMapXY(plane, tile_attr+2, x+1, y);
            VDP_setTileMapXY(plane, tile_attr+3, x+1, y+1);
        }
    }
}

void GFX_draw_sprite_in_plane_2x2(VDPPlane plane, u16 x, u16 y, u16 tile_attr) __attribute__((noinline));
