#pragma once

/*
This file is part of Blob Genesis.

Blob Genesis is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

Blob Genesis is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with Foobar. If not, see <https://www.gnu.org/licenses/>.
*/

#include "game_config.h"
#include <genesis.h>

enum InputMethod
{
    INPUT_METHOD_MOUSE,
    INPUT_METHOD_PAD
} ENUM_PACK;

enum InputCursor
{
    INPUT_CURSOR_NORMAL,
    INPUT_CURSOR_MOVE,
    INPUT_CURSOR_ROTATE_CW,
    INPUT_CURSOR_ROTATE_CCW,
    INPUT_CURSOR_HAMMER,
    INPUT_CURSOR_NO,
    INPUT_CURSOR_WATER,
    INPUT_CURSOR_FIRE
} ENUM_PACK;


void INPUT_init();
void INPUT_do_move_abs( s16 mx, s16 my, bool do_clamp );
void INPUT_step();

enum InputMethod INPUT_get_curr_method();
void INPUT_get_last_state(bool *out_method_change, u16 *out_changed, u16 *out_state); // clears change flags (which are set on events)

bool INPUT_is_mouse_present();
void INPUT_set_cursor_visible(bool visible);
void INPUT_set_cursor(enum InputCursor img);
bool INPUT_is_cursor_visible();
void INPUT_get_cursor_position( s16 *dest_x, s16 *dest_y );
void INPUT_center_cursor();

u8 INPUT_get_input_dev_icon( u16 port );
