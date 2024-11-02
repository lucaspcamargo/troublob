/*
This file is part of Blob Genesis.

Blob Genesis is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

Blob Genesis is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with Foobar. If not, see <https://www.gnu.org/licenses/>.
*/

#include "input.h"
#include "game_config.h"
#include "resources.h"
#include <genesis.h>

static u8 port_type_1, joy_type_1, port_type_2, joy_type_2;

static enum InputMethod curr_im;
static bool im_changed;
static u16 last_changed, last_state;

static Sprite *mouse_cursor = NULL;
static s16 mouse_x = 160;
static s16 mouse_y = 92;
static bool mouse_present = FALSE;
static bool is_mouse_1;
static bool is_mouse_2;
static s16 last_mx, last_my = -1;


void _INPUT_handler( u16 joy, u16 changed, u16 state)
{
    const u16 mouse_joy = mouse_present? (is_mouse_1? JOY_1 : JOY_2) : 0xffff;
    enum InputMethod im = (joy == mouse_joy)? INPUT_METHOD_MOUSE : INPUT_METHOD_PAD;

    last_changed = changed;
    last_state = state;

    im_changed = (im != curr_im);
    curr_im = im;

    if( DEBUG_INPUT )
    {
        char buf[20];
        memset(buf,0,20);
        sprintf(buf, "%02x %04x %04x", joy, changed, state);
        VDP_drawTextBG(BG_A, buf, 20, 0);
    }
}

void INPUT_init()
{
    port_type_1 = JOY_getPortType(PORT_1);
    if(port_type_1 == PORT_TYPE_PAD)
        JOY_setSupport(PORT_1, JOY_SUPPORT_6BTN);
    else
        JOY_setSupport(PORT_1, JOY_SUPPORT_MOUSE);

    port_type_2 = JOY_getPortType(PORT_2);
    if(port_type_2 == PORT_TYPE_PAD)
        JOY_setSupport(PORT_2, JOY_SUPPORT_6BTN);
    else
        JOY_setSupport(PORT_2, JOY_SUPPORT_MOUSE);

    JOY_update();

    joy_type_1 = JOY_getJoypadType(JOY_1);
    joy_type_2 = JOY_getJoypadType(JOY_2);

    if(DEBUG_INPUT)
    {
        char buf[40];
        sprintf(buf, "port1 %d joy1 %d, port2 %d joy2 %d", port_type_1, joy_type_1, port_type_2, joy_type_2);
        VDP_drawTextBG(BG_A, buf, 1, 27);
    }

    is_mouse_1 = ( joy_type_1==JOY_TYPE_MOUSE && port_type_1 == PORT_TYPE_MOUSE );
    is_mouse_2 = ( joy_type_2==JOY_TYPE_MOUSE && port_type_2 == PORT_TYPE_MOUSE );
    mouse_present = is_mouse_1 || is_mouse_2;

    // init cursor regardless
    mouse_cursor = SPR_addSprite(&spr_cursor, mouse_x, mouse_y, PAL_LINE_HUD<<TILE_ATTR_PALETTE_SFT);
    SPR_setPriority(mouse_cursor, TRUE);
    SPR_setDepth(mouse_cursor, SPR_MIN_DEPTH);
    SPR_setVisibility(mouse_cursor, HIDDEN);

    JOY_setEventHandler(&_INPUT_handler);
}

void INPUT_do_move_abs( s16 mx, s16 my, bool do_clamp )
{
    if(do_clamp)
    {
        mx = clamp(mx, MOUSE_MIN, MOUSE_X_MAX);
        my = clamp(my, MOUSE_MIN, MOUSE_Y_MAX);
    }

    if(mouse_cursor)
        SPR_setPosition(mouse_cursor, mx, my);

    mouse_x = mx;
    mouse_y = my;

    // update SGDK value to make clamp count internally
    u16 mouse_joy = is_mouse_2? JOY_2 : (is_mouse_1? JOY_1 : 0xffff);
    if (mouse_joy != 0xffff)
    {
        JOY_writeJoypadX(mouse_joy, mouse_x);
        JOY_writeJoypadY(mouse_joy, mouse_y);
    }

    if( DEBUG_INPUT )
    {
        char buf[40];
        memset(buf,0,40);
        sprintf(buf, "%d/%d @%d,%d",
                (int) JOY_getPortType(JOY_1),
                (int) JOY_getJoypadType(JOY_1),
                (int) mx,
                (int) my);
        VDP_drawTextBG(BG_A, buf, 0, 0);
    }
}

void INPUT_step()
{

    // Huh. Docs say mouse is relative BUT BlastEm gives me absolute coords. Hence this flag.
    u16 mouse_joy = is_mouse_2? JOY_2 : (is_mouse_1? JOY_1 : 0xffff);
    if (mouse_joy != 0xffff)
    {
        s16 mx = JOY_readJoypadX(mouse_joy);
        s16 my = JOY_readJoypadY(mouse_joy);

        if (mx != last_mx || my != last_my) // only move on change
            INPUT_do_move_abs(mx, my, TRUE);

        if( DEBUG_INPUT )
        {
            char buf[40];
            memset(buf,0,40);
            sprintf(buf, "%d/%d @%d,%d\n",
                    (int) JOY_getPortType(JOY_1),
                    (int) JOY_getJoypadType(JOY_1),
                    (int) mx,
                    (int) my);
            VDP_drawTextBG(BG_A, buf, 0, 0);
        }
        last_mx = mx;
        last_my = my;
    }
    /*
    else
    {
        s16 dx = JOY_readJoypadX(mouse_joy);
        s16 dy = JOY_readJoypadY(mouse_joy);

        dx += (jp&BUTTON_RIGHT?MOUSE_SPEED:0);
        dx += (jp&BUTTON_LEFT?-MOUSE_SPEED:0);
        dy += (jp&BUTTON_UP?-MOUSE_SPEED:0);
        dy += (jp&BUTTON_DOWN?MOUSE_SPEED:0);

        INPUT_do_move_abs(mouse_x+dx, mouse_y+dy, TRUE);
    }*/
}





enum InputMethod INPUT_get_curr_method()
{
    return curr_im;
}

void INPUT_get_last_state(bool *out_method_change, u16 *out_changed, u16 *out_state)
{
    (*out_method_change) = im_changed;
    (*out_changed) = last_changed;
    (*out_state) = last_state;

    last_changed = 0;
}



bool INPUT_is_mouse_present()
{
    return mouse_present;
}


void INPUT_set_cursor_visible(bool visible)
{
    if(mouse_cursor)
        SPR_setVisibility(mouse_cursor,visible?VISIBLE:HIDDEN);
}


void INPUT_set_cursor(enum InputCursor cur)
{
    if(mouse_cursor)
        SPR_setAnimAndFrame(mouse_cursor, 0, cur);
}

bool INPUT_is_cursor_visible()
{
    if(mouse_cursor)
        return SPR_getVisibility(mouse_cursor) == VISIBLE;
    else
        return FALSE;
}

void INPUT_get_cursor_position( s16 *dest_x, s16 *dest_y )
{
    *dest_x = mouse_x;
    *dest_y = mouse_y;
}


void INPUT_center_cursor()
{
    INPUT_do_move_abs(160, 112, TRUE);

}

u8 INPUT_get_input_dev_icon( u16 port )
{
    u8 port_type = port==PORT_1? port_type_1 : port_type_2;
    u8 joy_type = port==PORT_1? joy_type_1 : joy_type_2;

    if(port_type == PORT_TYPE_MOUSE && joy_type == JOY_TYPE_MOUSE)
        return 2;

    if(port_type == PORT_TYPE_PAD && joy_type == JOY_TYPE_PAD3)
        return 0;

    if(port_type == PORT_TYPE_PAD && joy_type == JOY_TYPE_PAD6)
        return 1;

    return 3;

}
