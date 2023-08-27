#include "gfx_utils.h"

void GFX_draw_sprite_in_plane_2x2(VDPPlane plane, u16 x, u16 y, u16 tile_attr)
{
    GFX_draw_sprite_in_plane_2x2_inline(plane, x, y, tile_attr );
}
