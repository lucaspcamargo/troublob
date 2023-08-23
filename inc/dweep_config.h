#pragma once

// Dweep Gen config file

// Metadata
#define GAME_NAME "DWEEP GENESIS"
#define GAME_VERSION "v 0.1"

// Debug Switches
#define DEBUG_CPU_LOAD 1
#define DEBUG_TILES 0
#define DEBUG_MAP_OBJS 0
#define DEBUG_MAP_PLANE_A_ALLOC 0
#define DEBUG_INPUT 0
#define DEBUG_CAMERA 0
#define DEBUG_PLAYER 0
#define DEBUG_PATHFINDING 0
#define DEBUG_PATHFINDING_FIELD 0
#define DEBUG_PATHFINDING_SLOW 0
#define FORCE_MOUSE 1
#define FORCE_MOUSE_ABSOLUTE 1

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

// Audio
#define SFX_USER_OFFSET 64

// Palette and scanout
#define PAL_LINE_HUD PAL0
#define PAL_LINE_SPR_A PAL1
#define PAL_LINE_BG_0 PAL2
#define PAL_LINE_BG_1 PAL3
#define FADE_DUR 15
#define RASTER_HUD_LINE (192)-43
#define RASTER_HUD_LINE_END (225)


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
