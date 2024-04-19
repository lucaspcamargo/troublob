#pragma once
#include "dweep_config.h"
#include <genesis.h>

enum PlayerState {
    PLR_STATE_IDLE = 0,
    PLR_STATE_MOVING_PATH,
    PLR_STATE_FROZEN,
    PLR_STATE_FLOATING,
    PLR_STATE_FLOATING_START,
    PLR_STATE_DYING,
    PLR_STATE_DEAD
} ENUM_PACK;

enum PlayerEmote {
    PLR_EMOTE_NEUTRAL = 0,
    PLR_EMOTE_HAPPY,
    PLR_EMOTE_FEAR,
    PLR_EMOTE_NONE
} ENUM_PACK;


void PLR_init();
void PLR_reset();
void PLR_destroy();

void PLR_reset_position();
bool PLR_goto(u16 dest_x, u16 dest_y); // in integer tile coords
u16 PLR_curr_tile_x();
u16 PLR_curr_tile_y();

enum PlayerState PLR_curr_state();

bool PLR_is_wet();
void PLR_wet();
void PLR_freeze();
void PLR_fire();
void PLR_float();
bool PLR_unfloat();

void PLR_update(u32 framecounter);
