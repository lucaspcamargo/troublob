#pragma once

#include <genesis.h>


void GFX_init_queue();
void GFX_stop_queue();
bool GFX_is_queued();
void GFX_queue_clear();

void GFX_draw_sprite_in_plane_2x2(VDPPlane plane, u16 x, u16 y, u16 tile_attr);
void GFX_draw_sprite_in_plane_2x2_immediate(VDPPlane plane, u16 x, u16 y, u16 tile_attr);

void GFX_clear_2x2(VDPPlane plane, u16 x, u16 y);
void GFX_clear_2x2_immediate(VDPPlane plane, u16 x, u16 y);
