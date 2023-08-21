#pragma once

#include <genesis.h>

#include "dweep_config.h"
#include "resources.h"

// HACK +256 for single bg tileset loaded at TILE_USER_INDEX
//      Need global mechanism for managing VDP tile memory in future, maybe
#define TILE_HUD_INDEX (TILE_USER_INDEX+256)

void HUD_draw();

void HUD_init();

