#include "player.h"

#include <genesis.h>
#include "resources.h"
#include "palette_ctrl.h"
#include "playfield.h"
#include "sfx.h"
#include "plf_obj.h"

static fix16 player_pf_x;
static fix16 player_pf_y;
static fix16 player_pf_z;
static fix16 dest_pf_x;
static fix16 dest_pf_y;
static fix16 final_dest_pf_x;
static fix16 final_dest_pf_y;
static u16 player_int_x; // integer tile position player is on
static u16 player_int_y;
static enum PlayerState player_state;
static enum PlayerEmote player_emote;
static u8 player_flags;
static Sprite *spr_player;
static Sprite *spr_player_shadow;
static Sprite *spr_player_eyes;

enum PlayerFlags {
    PLAYER_FLAG_NONE = 0,
    PLAYER_FLAG_WET = 1,
} ENUM_PACK;


#define TILE_IS_DANGEROUS(t) (t && (t->attrs & PLF_ATTR_DANGER || t->laser & PLF_LASER_OUT_ALL))

void _PLR_update_gfx(bool blink, u8 anim_frame);
void _PLR_update_bounce(u32 framecounter);
bool _PLR_check_danger(u8 dist);

// TODO remove these after fix in SGDK
FORCE_INLINE fix16 _fix16Round(fix16 value)
{
    return fix16Int(value + (FIX16(0.5)-1));
}

FORCE_INLINE s16 _fix16ToRoundedInt(fix16 value)
{
    return fix16ToInt(value + (FIX16(0.5)-1));
}

void PLR_init()
{
    // player
    PCTRL_set_line(PAL_LINE_SPR_A, spr_dweep.palette->data);
    spr_player = SPR_addSprite(&spr_dweep, 0, 0, PAL_LINE_SPR_A<<TILE_ATTR_PALETTE_SFT);
    spr_player_shadow = SPR_addSprite(&spr_shadow, 0, 0, PAL_LINE_SPR_A<<TILE_ATTR_PALETTE_SFT);
    spr_player_eyes = SPR_addSprite(&spr_dweep_eyes, 0, 0, PAL_LINE_SPR_A<<TILE_ATTR_PALETTE_SFT);
    SPR_setPriority(spr_player_shadow, FALSE);
    SPR_setVisibility(spr_player, VISIBLE);

    player_pf_x = 0;
    player_pf_y = 0;
    player_pf_z = FIX16(0);
    dest_pf_x = player_pf_x;
    dest_pf_y = player_pf_y;
    final_dest_pf_x = player_pf_x;
    final_dest_pf_y = player_pf_y;
    player_int_x = fix16ToInt(player_pf_x);
    player_int_y = fix16ToInt(player_pf_y);

    player_state = PLR_STATE_IDLE;
    player_emote = PLR_EMOTE_NEUTRAL;
    player_flags = PLAYER_FLAG_NONE;
    _PLR_update_gfx(FALSE, 0);
}

void PLR_reset()
{
    PLR_destroy();
    PLR_init();
}

void PLR_destroy()
{
    SPR_releaseSprite(spr_player);
    SPR_releaseSprite(spr_player_shadow);
    SPR_releaseSprite(spr_player_eyes);
}

void PLR_reset_position()
{
    PLF_player_get_initial_pos(&player_pf_x, &player_pf_y);
    player_pf_z = FIX16(0);
    dest_pf_x = player_pf_x;
    dest_pf_y = player_pf_y;
    final_dest_pf_x = player_pf_x;
    final_dest_pf_y = player_pf_y;
    player_int_x = fix16ToInt(player_pf_x);
    player_int_y = fix16ToInt(player_pf_y);
}



bool PLR_goto(u16 dest_x, u16 dest_y)
{
    if(player_state != PLR_STATE_IDLE && player_state != PLR_STATE_MOVING_PATH)
    {
        SFX_play(SFX_no);
        return FALSE;
    }

    bool found = PLF_player_pathfind(fix16ToInt(player_pf_x),fix16ToInt(player_pf_y), dest_x, dest_y);
    if (found)
    {
        SFX_play(SFX_go1+(random()%3));
        player_state = PLR_STATE_MOVING_PATH;
        final_dest_pf_x = FIX16(dest_x);
        final_dest_pf_y = FIX16(dest_y);
        u16 bufx, bufy;
        bool found_next_pos = PLF_player_path_next(fix16ToInt(player_pf_x), fix16ToInt(player_pf_y), &bufx, &bufy);
        if(found_next_pos)
        {
            dest_pf_x = FIX16(bufx);
            dest_pf_y = FIX16(bufy);
            return TRUE;
        }
        else
        {
            // this not supposed to happen, given PLF_player_pathfind succeeded
            return FALSE;
        }
    }
    else
    {
        SFX_play(SFX_no);
        return FALSE;
    }
}


u16 PLR_curr_tile_x()
{
    return player_int_x;
}

u16 PLR_curr_tile_y()
{
    return player_int_y;
}

enum PlayerState PLR_curr_state()
{
    return player_state;
}

bool PLR_is_wet()
{
    return player_flags & PLAYER_FLAG_WET? TRUE : FALSE;
}

void PLR_wet()
{
    player_flags |= PLAYER_FLAG_WET;
    SFX_play(SFX_water);
}

void PLR_freeze()
{
    player_state = PLR_STATE_FROZEN;
    player_pf_x = intToFix16(player_int_x);
    player_pf_y = intToFix16(player_int_y);
    player_pf_z = FIX16(0);
    dest_pf_x = player_pf_x;
    dest_pf_y = player_pf_y;
    final_dest_pf_x = player_pf_x;
    final_dest_pf_y = player_pf_y;
}


void PLR_fire()
{
    if(player_state == PLR_STATE_FROZEN)
    {
        player_state = PLR_STATE_IDLE;
        player_flags |= PLAYER_FLAG_WET;
        SFX_play(SFX_thaw);
    }
    else
    {
        player_state = PLR_STATE_DYING;
        SFX_play(SFX_burn);
    }
}

void PLR_float()
{
    player_state = PLR_STATE_FLOATING_START;
}

bool PLR_unfloat()
{
    // TODO impl
    return FALSE;
}


void _PLR_update_bounce(u32 framecounter)
{
    if(player_state == PLR_STATE_DYING || player_state == PLR_STATE_DEAD || player_state == PLR_STATE_FROZEN)
        return;
    int angle = (framecounter * 4)&1023;
    /*if(player_float)
    {
        player_pf_z = FIX16(6)+fix16Mul(sinFix16(angle/2), FIX16(4));
    }*/

    player_pf_z = fix16Mul(sinFix16(angle*2), FIX16(6));
    player_pf_z = player_pf_z<0?fix16Neg(player_pf_z):player_pf_z;
}

bool _PLR_check_danger(u8 dist)
{
    // see if there are dangerous things within "radius" (square centered around the player)
    // TODO move this to playfield code, optimize, reenable
    return FALSE;

    u16 curr_x = player_int_x - dist;
    u16 curr_y = player_int_y - dist;
    u8 side_len = 1 + 2*dist;
    for(int i = 0; i < (side_len-1); i++)
    {
        curr_x++;
        PlfTile * t = PLF_get_tile_safe(curr_x, curr_y);
        if(TILE_IS_DANGEROUS(t))
            return TRUE;
    }
    for(int i = 0; i < (side_len-1); i++)
    {
        curr_y++;
        PlfTile * t = PLF_get_tile_safe(curr_x, curr_y);
        if(TILE_IS_DANGEROUS(t))
            return TRUE;
    }
    for(int i = 0; i < (side_len-1); i++)
    {
        curr_x--;
        PlfTile * t = PLF_get_tile_safe(curr_x, curr_y);
        if(TILE_IS_DANGEROUS(t))
            return TRUE;
    }
    for(int i = 0; i < (side_len-1); i++)
    {
        curr_y--;
        PlfTile * t = PLF_get_tile_safe(curr_x, curr_y);
        if(TILE_IS_DANGEROUS(t))
            return TRUE;
    }

    return (dist == 1? FALSE : _PLR_check_danger(dist-1));
}


void _PLR_update_gfx(bool blink_frame, u8 anim_frame)
{
    // player
    u16 px = _fix16ToRoundedInt(fix16Mul(player_pf_x, FIX16(16)))-4;
    u16 py = _fix16ToRoundedInt(fix16Sub(fix16Mul(player_pf_y, FIX16(16)), player_pf_z))-10;
    u16 depth = PLF_get_sprite_depth(player_pf_x, player_pf_y);
    SPR_setPosition(spr_player, px, py);
    SPR_setDepth(spr_player, depth);
    SPR_setAnimAndFrame(spr_player, 0, anim_frame);

    // shadow
    SPR_setPosition(spr_player_shadow,
        _fix16ToRoundedInt(fix16Mul(player_pf_x, FIX16(16))),
        _fix16ToRoundedInt(fix16Mul(player_pf_y, FIX16(16)))+4+3);
    SPR_setFrame(spr_player_shadow, max(0,min(3, fix16ToRoundedInt(player_pf_z)/2-1)) );


    // eyes
    if(player_emote == PLR_EMOTE_NONE || (player_emote == PLR_EMOTE_NEUTRAL && !blink_frame))
    {
        SPR_setVisibility(spr_player_eyes, HIDDEN);
    }else{
        SPR_setVisibility(spr_player_eyes, VISIBLE);
        SPR_setFrame(spr_player_eyes, player_emote == PLR_EMOTE_FEAR? 1 : 0);
        SPR_setPosition(spr_player_eyes, px+4, py+6);
        SPR_setDepth(spr_player_eyes, depth - 1);
    }

}


void PLR_update(u32 framecounter)
{

    if(player_state == PLR_STATE_MOVING_PATH)
    {
        u8 axii_changed = 0;
        if(player_pf_x != dest_pf_x)
        {
            fix16 delta = ((player_pf_x < dest_pf_x)?PLAYER_SPEED:-PLAYER_SPEED);
            player_pf_x += delta;
            axii_changed++;
            //SPR_setHFlip(spr_player, delta<0);
        }
        if(player_pf_y != dest_pf_y)
        {
            player_pf_y += ((player_pf_y < dest_pf_y)?PLAYER_SPEED:-PLAYER_SPEED);
            axii_changed++;
        }

        bool int_changed = FALSE;
        u16 prev_int_x = player_int_x;
        u16 prev_int_y = player_int_y;
        if(axii_changed)
        {
            const s16 rx = _fix16ToRoundedInt(player_pf_x);
            const s16 ry = _fix16ToRoundedInt(player_pf_y);
            if((axii_changed == 2 && rx != player_int_x && ry != player_int_y) ||   // moving diagonally and changed both axii
                ((rx != player_int_x || ry != player_int_y) && axii_changed != 2))  // not moving diagonally, and any axis has changed
            {
                player_int_x = rx;
                player_int_y = ry;
                int_changed = TRUE;
            }
        }

        if (int_changed)
        {
            // arrived on a tile
            const PlfTile *tile = PLF_get_tile_safe(player_int_x, player_int_y);
            const PlfTile *prev_tile = PLF_get_tile_safe(prev_int_x, prev_int_y);


            if(tile && tile->laser)
            {
                PLF_laser_block(player_int_x, player_int_y);
                if(player_flags & PLAYER_FLAG_WET)
                {
                    player_flags &= ~PLAYER_FLAG_WET;
                    SFX_play(SFX_dry);
                }
                else
                {
                    player_state = PLR_STATE_DYING;
                    SFX_play(SFX_burn);
                }
            }

            if(tile && tile->pobj)
                Pobj_event(Pobj_get_data(tile->pobj), POBJ_EVT_STEPPED, NULL);

            if(prev_tile && prev_tile->laser)
            {
                PLF_laser_recalc(prev_int_x, prev_int_y);
            }
        }

        // TODO instead of doing this, keep track of which tile dweep is actually in, and then act on that
        // use int_changed above
        if(player_state == PLR_STATE_MOVING_PATH && axii_changed && fix16Frac(player_pf_x)==0 && fix16Frac(player_pf_y)==0)
        {

            if(player_pf_x == dest_pf_x && player_pf_y == dest_pf_y)
            {
                // arrived at immediate dest, either we arrived at final destination, or we find our next imm. dest.
                if(player_pf_x == final_dest_pf_x && player_pf_y == final_dest_pf_y)
                {
                    player_state = PLR_STATE_IDLE;
                }
                else
                {
                    u16 bufx, bufy;
                    bool found_next_pos = PLF_player_path_next(fix16ToInt(player_pf_x), fix16ToInt(player_pf_y), &bufx, &bufy);
                    if(found_next_pos)
                    {
                        dest_pf_x = FIX16(bufx);
                        dest_pf_y = FIX16(bufy);
                    }
                    else player_state = PLR_STATE_IDLE;
                }
            }
        }
    }

    // check player emotions
    if (_PLR_check_danger(1))
        player_emote = PLR_EMOTE_FEAR;
    else
        player_emote = _PLR_check_danger(3)? PLR_EMOTE_NEUTRAL : PLR_EMOTE_HAPPY;

    bool upward = ((framecounter*8) % 512) >= 200? 0 : 1;
    u16 anim_frame = upward;
    if(player_flags & PLAYER_FLAG_WET)
    {
        anim_frame += 4;
        if(!(framecounter%64))
            SFX_play(SFX_mop);
    }
    if(player_state == PLR_STATE_DYING || player_state == PLR_STATE_DEAD)
    {
        anim_frame = 2;
        player_emote = PLR_EMOTE_NONE;
        player_state = PLR_STATE_DEAD;  // TODO implement dying animations
    }
    else if(player_state == PLR_STATE_FROZEN)
    {
        anim_frame = 3;
        player_emote = PLR_EMOTE_NONE;
    }

    _PLR_update_bounce(framecounter);
    _PLR_update_gfx((framecounter%128)<6, anim_frame);

    if(DEBUG_PLAYER)
    {
        char buf[20];
        sprintf(buf, "S%X@%X,%X,%X     ",
                (int) player_state,
                (int) player_pf_x, (int) player_pf_y, (int) player_pf_z);
        VDP_drawTextBG(WINDOW, buf, 0, 24);
        sprintf(buf, "%X,%X     ", (int) player_int_x, (int) player_int_y);
        VDP_drawTextBG(WINDOW,buf, 20, 24);
        //sprintf(buf, "%X,%X     ", (int) dest_pf_x, (int) dest_pf_y);
        //VDP_drawTextBG(WINDOW,buf, 20, 24);
        sprintf(buf, "%X,%X     ", (int) final_dest_pf_x, (int) final_dest_pf_y);
        VDP_drawTextBG(WINDOW,buf, 30, 24);
    }
}
