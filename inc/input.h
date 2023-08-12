#pragma once
#include <genesis.h>

// joypad/mouse initialization and input handling functions

void INPUT_handler( u16 joy, u16 changed, u16 state);
void INPUT_init();
void INPUT_do_move_abs( s16 mx, s16 my, bool do_clamp );
bool INPUT_step();
void INPUT_set_cursor_visible(bool visible);
bool INPUT_is_cursor_visible();
void INPUT_get_cursor_position( s16 *dest_x, s16 *dest_y );
