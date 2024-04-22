#pragma once

/*
This file is part of Dweep Genesis.

Dweep Genesis is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

Dweep Genesis is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with Foobar. If not, see <https://www.gnu.org/licenses/>.
*/


// Dweep Gen config file

// Metadata
#define GAME_NAME "DWEEP GENESIS"
#define GAME_VERSION "v 0.1"

// Debug Switches
#if DEBUG == 1
#define DEBUG_MENU 1
#define DEBUG_CPU_LOAD 1
#else
#define DEBUG_MENU 1
#define DEBUG_CPU_LOAD 0
#endif
#define DEBUG_TILES 0
#define DEBUG_MAP_OBJS 0
#define DEBUG_MAP_PLANE_A_ALLOC 0
#define DEBUG_INPUT 0
#define DEBUG_CAMERA 0
#define DEBUG_PLAYER 0
#define DEBUG_PATHFINDING 0
#define DEBUG_PATHFINDING_FIELD 0
#define DEBUG_PATHFINDING_SLOW 0
#define DEBUG_LASER_BEHAVIOR 0

// Feature switches
#define DISABLE_SFX 0

// Game constants
#define PLAYFIELD_VIEW_W 20
#define PLAYFIELD_VIEW_H 14
#define PLAYFIELD_STD_SZ (PLAYFIELD_VIEW_W*PLAYFIELD_VIEW_H)
#define PLAYER_SPEED 4
#define MOUSE_MIN 1
#define MOUSE_X_MAX 318
#define MOUSE_Y_MAX 222
#define MOUSE_SPEED 2

// Save
#define SAVE_NUM_SLOTS 4
#define SAVE_COMPLETION_ENTRIES_COUNT 32
#define SAVE_CHECKSUM_POLY 0x8005

// Audio
#define SFX_USER_OFFSET 64

// Palettes
#define PAL_LINE_HUD PAL0
#define PAL_LINE_SPR_A PAL1
#define PAL_LINE_BG_0 PAL2
#define PAL_LINE_BG_1 PAL3
#define PAL_STD_FADE_DURATION 16

// Utility macros
#define ENUM_PACK __attribute__((packed))  // add to end of enum declaration to use smallest possible type
#define DIR_R 0
#define DIR_L 1
#define DIR_U 2
#define DIR_D 3

#define DIR_UPDATE_XY(xvar, yvar, dir) {\
    if(dir == DIR_R) xvar += 1;\
    if(dir == DIR_L) xvar -= 1;\
    if(dir == DIR_U) yvar -= 1;\
    if(dir == DIR_D) yvar += 1;\
}

#define DIR_IS_VERTICAL(dir) (dir == DIR_U || dir == DIR_D)
#define DIR_IS_HORIZONTAL(dir) (dir == DIR_R || dir == DIR_L)
#define DIR_OPPOSITE(dir) (dir ^ 0x01)  // flip LSB

inline unsigned char DIR_ROT_CW(unsigned char dir) {
    switch(dir)
    {
        case DIR_R:
            return DIR_D;
        case DIR_D:
            return DIR_L;
        case DIR_L:
            return DIR_U;
        case DIR_U:
            return DIR_R;
    }
    return 0xff;
}

inline unsigned char DIR_ROT_CCW(unsigned char dir) {
    switch(dir)
    {
        case DIR_R:
            return DIR_U;
        case DIR_U:
            return DIR_L;
        case DIR_L:
            return DIR_D;
        case DIR_D:
            return DIR_R;
    }
    return 0xff;
}
