#pragma once

// Dweep Gen config file

// Metadata
#define GAME_NAME "DWEEP GENESIS"
#define GAME_VERSION "v 0.1"

// Debug Switchews
#define DEBUG_METATILES 0
#define DEBUG_MOUSE 0
#define DEBUG_CAMERA 0
#define FORCE_MOUSE 1

// Game constants
#define PLAYFIELD_STD_W 20
#define PLAYFIELD_STD_H 12
#define PLAYFIELD_STD_SZ (PLAYFIELD_STD_W*PLAYFIELD_STD_H)
#define PLAYER_SPEED 2
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
