#pragma once

#include <genesis.h>

#include "dweep_config.h"
#include "resources.h"
#include "tools.h"

enum HUDState
{
    HUD_ST_NORMAL,              // normal gameplay state
    HUD_ST_DIALOG_STARTING,     // drawing the dialog background and portrait
    HUD_ST_DIALOG_TYPING,       // drawing the works animatedly
    HUD_ST_DIALOG_IDLE,         // dialog written
    HUD_ST_UNINITIALIZED = 0xff
} ENUM_PACK;

#define HUD_Y_TILE 24
#define HUD_Y_POS (HUD_Y_TILE * 8)

void HUD_preinit(); // just load graphics data used by rest of system
void HUD_init();
void HUD_set_visible(bool visible);
void HUD_update();

void HUD_dialog_start(u8 character_id, u16 string_id);
void HUD_dialog_end();

void HUD_inventory_set(const enum ToolId * tool_arr);
void HUD_inventory_set_curr_idx(u8 idx);
bool HUD_inventory_push(enum ToolId tool);
void HUD_inventory_pop(u8 idx);
void HUD_inventory_pop_curr();
void HUD_inventory_clear();
u16  HUD_inventory_count();
void HUD_inventory_sel_next();
void HUD_inventory_sel_prev();
enum ToolId HUD_inventory_curr();

void HUD_on_click(s16 mouse_x, s16 mouse_y);
