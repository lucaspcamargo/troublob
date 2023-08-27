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

bool PLR_goto(f16 dest_x, f16 dest_y);

void PLR_update(u32 framecounter);
