#include "input.h"
#include "dweep_config.h"
#include "resources.h"
#include <genesis.h>


static Sprite *mouse_cursor;
static s16 mouse_x = 160;
static s16 mouse_y = 92;
static bool mouse_present = FALSE;
static u16 mouse_last_joy_state = 0;
static bool mouse_last_clicked = FALSE;
static bool is_mouse_1;
static bool is_mouse_2;
static s16 last_mx, last_my = -1;

#define MOUSE_DIV (FORCE_MOUSE_ABSOLUTE?2:1)   // HACK DIVIDING MOUSE INPUT BY 2 TO GET PARITY WITH EMULATOR WINDOW SZ


void INPUT_handler( u16 joy, u16 changed, u16 state)
{
    // just save curr state for movement handling in callback
    if(joy == JOY_1 || joy==JOY_2)
    {
        mouse_last_joy_state = state;
        if(changed & state & BUTTON_A)
            mouse_last_clicked = TRUE;

        if( DEBUG_INPUT )
        {
            char buf[20];
            memset(buf,0,20);
            sprintf(buf, "%02x %04x %04x", joy, changed, state);
            VDP_drawTextBG(BG_A, buf, 20, 0);
        }
    }
}

void INPUT_init()
{
    //JOY_setSupport(PORT_1, JOY_SUPPORT_3BTN|JOY_SUPPORT_6BTN|JOY_SUPPORT_MOUSE);
    u8 port_type_1 = JOY_getPortType(PORT_1);
    u8 joy_type_1 = JOY_getJoypadType(JOY_1);
    u8 port_type_2 = JOY_getPortType(PORT_2);
    u8 joy_type_2 = JOY_getJoypadType(JOY_2);

    if(DEBUG_INPUT)
    {
        char buf[40];
        sprintf(buf, "port1 %d joy1 %d, port2 %d joy2 %d", port_type_1, joy_type_1, port_type_2, joy_type_2);
        VDP_drawTextBG(BG_A, buf, 1, 27);
    }

    is_mouse_1 = ( joy_type_1==JOY_TYPE_MOUSE && port_type_1 == PORT_TYPE_MOUSE );
    is_mouse_2 = ( joy_type_2==JOY_TYPE_MOUSE && port_type_2 == PORT_TYPE_MOUSE );

    if( FORCE_MOUSE || is_mouse_1 || is_mouse_2)
    {
        mouse_present = TRUE;
        mouse_cursor = SPR_addSprite(&spr_cursor, mouse_x, mouse_y, PAL_LINE_SPR_A<<TILE_ATTR_PALETTE_SFT);
        SPR_setPriority(mouse_cursor, TRUE);
    }

    JOY_setEventHandler(&INPUT_handler);
}

void INPUT_do_move_abs( s16 mx, s16 my, bool do_clamp )
{
    if(do_clamp)
    {
        mx = clamp(mx, MOUSE_MIN, MOUSE_X_MAX);
        my = clamp(my, MOUSE_MIN, MOUSE_Y_MAX);
    }
    SPR_setPosition(mouse_cursor, mx, my);
    mouse_x = mx; mouse_y = my;
    if( DEBUG_INPUT )
    {
        char buf[40];
        memset(buf,0,40);
        sprintf(buf, "%d/%d @%d,%d\n",
                (int) JOY_getPortType(JOY_1),
                (int) JOY_getJoypadType(JOY_1),
                (int) mx,
                (int) my);
        VDP_drawTextBG(BG_A, buf, 0, 1);
    }
}

bool INPUT_step()
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
    u16 move_joy_tgt = is_mouse_2? JOY_2 : (is_mouse_1? JOY_1 : 0xffff); // still force at least joy_1 for mouse control
    if (move_joy_tgt != 0xffff)
    {
        if(FORCE_MOUSE_ABSOLUTE)
        {
            s16 mx = JOY_readJoypadX(move_joy_tgt);
            s16 my = JOY_readJoypadY(move_joy_tgt);

            if (mx != last_mx || my != last_my) // only move on change
                INPUT_do_move_abs(mx/MOUSE_DIV, my/MOUSE_DIV, TRUE);

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
        else
        {
            s16 dx = JOY_readJoypadX(move_joy_tgt);
            s16 dy = JOY_readJoypadY(move_joy_tgt);
            u16 jp = mouse_last_joy_state;

            dx += (jp&BUTTON_RIGHT?MOUSE_SPEED:0);
            dx += (jp&BUTTON_LEFT?-MOUSE_SPEED:0);
            dy += (jp&BUTTON_UP?-MOUSE_SPEED:0);
            dy += (jp&BUTTON_DOWN?MOUSE_SPEED:0);

            INPUT_do_move_abs(mouse_x+dx, mouse_y+dy, TRUE);
        }
    }

    return ret;
}

void INPUT_set_cursor_visible(bool visible)
{
    SPR_setVisibility(mouse_cursor,visible?VISIBLE:HIDDEN);
}

bool INPUT_is_cursor_visible()
{
    return SPR_getVisibility(mouse_cursor);
}

void INPUT_get_cursor_position( s16 *dest_x, s16 *dest_y )
{
    *dest_x = mouse_x;
    *dest_y = mouse_y;
}
