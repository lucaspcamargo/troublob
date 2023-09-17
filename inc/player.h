#pragma once
#include "dweep_config.h"
#include <genesis.h>

enum PlayerState {
    PLR_STATE_IDLE = 0,
    PLR_STATE_MOVING_PATH
} ENUM_PACK;

enum PlayerEmote {
    PLR_EMOTE_NEUTRAL = 0,
    PLR_EMOTE_HAPPY,
    PLR_EMOTE_FEAR
} ENUM_PACK;


void PLR_init();
void PLR_reset_position();
bool PLR_goto(u16 dest_x, u16 dest_y); // in integer tile coords
u16 PLR_curr_tile_x();
u16 PLR_curr_tile_y();
void PLR_update(u32 framecounter);
