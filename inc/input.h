#pragma once
#include "dweep_config.h"
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
    INPUT_CURSOR_DEATH
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

u8 INPUT_get_input_dev_icon( u16 port );
