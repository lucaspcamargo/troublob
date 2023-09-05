#include "player.h"

#include <genesis.h>
#include "resources.h"
#include "palette_ctrl.h"
#include "playfield.h"
#include "sfx.h"

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
static Sprite *spr_player;
static Sprite *spr_player_shadow;
static Sprite *spr_player_eyes;


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
    PCTRL_set_source(PAL_LINE_SPR_A, spr_dweep.palette->data);
    spr_player = SPR_addSprite(&spr_dweep, 0, 0, PAL_LINE_SPR_A<<TILE_ATTR_PALETTE_SFT);
    spr_player_shadow = SPR_addSprite(&spr_shadow, 0, 0, PAL_LINE_SPR_A<<TILE_ATTR_PALETTE_SFT);
    spr_player_eyes = SPR_addSprite(&spr_dweep_eyes, 0, 0, PAL_LINE_SPR_A<<TILE_ATTR_PALETTE_SFT);
    SPR_setPriority(spr_player_shadow, FALSE);
    SPR_setVisibility(spr_player, VISIBLE);

    PLF_player_get_initial_pos(&player_pf_x, &player_pf_y);
    player_pf_z = FIX16(0);
    dest_pf_x = player_pf_x;
    dest_pf_y = player_pf_y;
    final_dest_pf_x = player_pf_x;
    final_dest_pf_y = player_pf_y;
    player_int_x = fix16Int(player_pf_x);
    player_int_y = fix16Int(player_pf_y);
    player_state = PLR_STATE_IDLE;
    player_emote = PLR_EMOTE_NEUTRAL;

    _PLR_update_gfx(FALSE, 0);

}

bool PLR_goto(f16 dest_x, f16 dest_y)
{
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

void _PLR_update_bounce(u32 framecounter)
{
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
    SPR_setFrame(spr_player_shadow, \
    ((s16) (fix16Div(player_pf_z, FIX16(3)) >> FIX16_FRAC_BITS)) );


    // eyes
    if(player_emote == PLR_EMOTE_NEUTRAL && !blink_frame)
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

    _PLR_update_bounce(framecounter);

    if(player_state == PLR_STATE_MOVING_PATH)
    {
        bool changed = FALSE;
        if(player_pf_x != dest_pf_x)
        {
            fix16 delta = ((player_pf_x < dest_pf_x)?PLAYER_SPEED:-PLAYER_SPEED);
            player_pf_x += delta;
            changed = TRUE;
            //SPR_setHFlip(spr_player, delta<0);
        }
        if(player_pf_y != dest_pf_y)
        {
            player_pf_y += ((player_pf_y < dest_pf_y)?PLAYER_SPEED:-PLAYER_SPEED);
            changed = TRUE;
        }

        bool int_changed = FALSE;
        if(changed)
        {
            // FIXME sometimes when moving diagonally,
            // we change only x or only y on a single frame?
            const s16 rx = _fix16ToRoundedInt(player_pf_x);
            const s16 ry = _fix16ToRoundedInt(player_pf_y);
            if(rx != player_int_x || ry != player_int_y)
            {
                player_int_x = rx;
                player_int_y = ry;
                int_changed = TRUE;
            }
        }

        // TODO instead of doing this, keep track of which tile dweep is actually in, and then act on that
        // use int_changed above
        (void) int_changed;
        if(changed && fix16Frac(player_pf_x)==0 && fix16Frac(player_pf_y)==0)
        {
            // arrived on a tile
            //PlfTile tile = *PLF_get_tile(fix16ToInt(player_pf_x), fix16ToInt(player_pf_y));
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


    _PLR_update_gfx((framecounter%128)<6, ((framecounter*8) % 512) >= 200? 0 : 1);

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
