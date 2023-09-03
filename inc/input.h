#pragma once
#include <genesis.h>

// joypad/mouse initialization and input handling functions

void INPUT_init();
void INPUT_do_move_abs( s16 mx, s16 my, bool do_clamp );
void INPUT_step();

bool INPUT_is_mouse_present();
void INPUT_set_cursor_visible(bool visible);
bool INPUT_is_cursor_visible();
void INPUT_get_cursor_position( s16 *dest_x, s16 *dest_y );

u8 INPUT_get_input_dev_icon( u16 port );
