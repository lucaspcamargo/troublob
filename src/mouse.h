#pragma once

#include <genesis.h>
#include "sfx.h"

Sprite *mouse_cursor;
s16 mouse_x = 160;
s16 mouse_y = 92;
bool mouse_present = FALSE;
u16 mouse_last_joy_state = 0;
bool mouse_last_clicked = FALSE;

void MOUSE_do_move_delta( s16 dx, s16 dy );

void MOUSE_handler( u16 joy, u16 changed, u16 state)
{
    // just save curr state for movement handling in callback
    if(joy == JOY_1)
    {
        mouse_last_joy_state = state;
        if(changed & state & BUTTON_A)
            mouse_last_clicked = TRUE;
    }

}

void MOUSE_init()
{
    JOY_setSupport(PORT_1, JOY_SUPPORT_MOUSE);
    u8 port_type = JOY_getPortType(PORT_1);
    u8 joy_type = JOY_getJoypadType(JOY_1);

    if(DEBUG_MOUSE)
    {
        char buf[40];
        sprintf(buf, "port %d joy %d", port_type, joy_type);
        VDP_drawTextBG(BG_A, buf, 0, 27);
    }

    if( FORCE_MOUSE || ( joy_type==JOY_TYPE_MOUSE && port_type == PORT_TYPE_MOUSE ))
    {
        mouse_present = TRUE;
        mouse_cursor = SPR_addSprite(&spr_cursor, mouse_x, mouse_y, PAL_LINE_SPR_A<<TILE_ATTR_PALETTE_SFT);
        SPR_setPriority(mouse_cursor, TRUE);
    }

    JOY_setEventHandler(&MOUSE_handler);
}

#define MOUSE_do_move_delta( dx, dy ) MOUSE_do_move_abs(mouse_x+dx, mouse_y+dx)

void MOUSE_do_move_abs( s16 mx, s16 my, bool do_clamp )
{
    if(do_clamp)
    {
        mx = clamp(mx, MOUSE_MIN, MOUSE_X_MAX);
        my = clamp(my, MOUSE_MIN, MOUSE_Y_MAX);
    }
    SPR_setPosition(mouse_cursor, mx, my);
    mouse_x = mx; mouse_y = my;
    if( DEBUG_MOUSE )
    {
        char buf[40];
        memset(buf,0,40);
        sprintf(buf, "%d/%d @%d,%d\n",
                (int) JOY_getPortType(JOY_1),
                (int) JOY_getJoypadType(JOY_1),
                (int) mx,
                (int) my);
        VDP_drawText(buf, 0, 1);
    }
}

bool MOUSE_step()
{
    if(!mouse_present)
        return FALSE;

    bool ret = FALSE;
    if(mouse_last_clicked)
    {
        mouse_last_clicked = FALSE;
        ret = TRUE;
    }

    // Huh. Docs say mouse is relative BUT BlastEm gives me absolute coords. Hence this flag.
#if FORCE_MOUSE_ABSOLUTE
    s16 mx = JOY_readJoypadX(JOY_1);
    s16 my = JOY_readJoypadY(JOY_1);
    MOUSE_do_move_abs(mx/2, my/2, TRUE); // HACK DIVIDING BY 2 HERE TO GET PARITY WITH EMULATOR WINDOW SZ
#else
    s16 dx = JOY_readJoypadX(JOY_1);
    s16 dy = JOY_readJoypadY(JOY_1);
    u16 jp = mouse_last_joy_state;

    dx += (jp&BUTTON_RIGHT?MOUSE_SPEED:0);
    dx += (jp&BUTTON_LEFT?-MOUSE_SPEED:0);
    dy += (jp&BUTTON_UP?-MOUSE_SPEED:0);
    dy += (jp&BUTTON_DOWN?MOUSE_SPEED:0);


    if (dx || dy)
        MOUSE_do_move_delta(dx, dy);
#endif

    return ret;
}

void MOUSE_set_visible(bool visible)
{
    SPR_setVisibility(mouse_cursor,visible?VISIBLE:HIDDEN);
}
