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
    JOY_init();
    JOY_setSupport(PORT_1, JOY_SUPPORT_MOUSE);
    JOY_reset();
    JOY_setEventHandler(&MOUSE_handler);

    if( FORCE_MOUSE ||
        JOY_getJoypadType(JOY_1)==JOY_TYPE_MOUSE ||
        JOY_getPortType(PORT_1)==PORT_TYPE_MOUSE)
    {
        mouse_present = TRUE;
        mouse_cursor = SPR_addSprite(&spr_cursor, mouse_x, mouse_y, PAL_LINE_SPR_A<<TILE_ATTR_PALETTE_SFT);
        SPR_setPriority(mouse_cursor, TRUE);
    }
}

void MOUSE_do_move_delta( s16 dx, s16 dy )
{
    s16 mouseX = mouse_x + dx;
    s16 mouseY = mouse_y + dy;
    if (mouseY < MOUSE_MIN)
        mouseY = MOUSE_MIN;
    if (mouseX < MOUSE_MIN)
        mouseX = MOUSE_MIN;
    if (mouseY > MOUSE_Y_MAX)
        mouseY = MOUSE_Y_MAX;
    if (mouseX > MOUSE_X_MAX)
        mouseX = MOUSE_X_MAX;
    SPR_setPosition(mouse_cursor, mouseX, mouseY);
    mouse_x = mouseX; mouse_y = mouseY;
    if( DEBUG_MOUSE )
    {
        char buf[40];
        memset(buf,0,40);
        sprintf(buf, "%d/%d @%d,%d\n",
                (int) JOY_getPortType(JOY_1),
                (int) JOY_getJoypadType(JOY_1),
                (int) mouseX,
                (int) mouseY);
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

    s16 dx = JOY_readJoypadX(dx);
    s16 dy = JOY_readJoypadY(dy);
    u16 jp = mouse_last_joy_state;

    dx += (jp&BUTTON_RIGHT?MOUSE_SPEED:0);
    dx += (jp&BUTTON_LEFT?-MOUSE_SPEED:0);
    dy += (jp&BUTTON_UP?-MOUSE_SPEED:0);
    dy += (jp&BUTTON_DOWN?MOUSE_SPEED:0);


    if (dx || dy)
        MOUSE_do_move_delta(dx, dy);

    return ret;
}

void MOUSE_set_visible(bool visible)
{
    SPR_setVisibility(mouse_cursor,visible?VISIBLE:HIDDEN);
}
