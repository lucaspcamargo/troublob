#pragma once

/*
This file is part of Dweep Genesis.

Dweep Genesis is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

Dweep Genesis is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with Foobar. If not, see <https://www.gnu.org/licenses/>.
*/

#include "dweep_config.h"
#include "registry.h"
#include <genesis.h>


enum SaveInitStatus
{
    SAVE_INIT_OK,
    SAVE_INIT_CORRUPTED,
    SAVE_INIT_ALL_NEW,
    SAVE_INIT_UNINITIALIZED
} ENUM_PACK;

enum SaveSlotState
{
    SAVE_SLOT_NEW,
    SAVE_SLOT_USED,
    SAVE_SLOT_CORRUPTED,
    SAVE_SLOT_INVALID
} ENUM_PACK;


typedef struct GlobalOptions_st
{
    u8 lang;
    bool enable_music;
    bool enable_sfx;
    bool show_hints;
    bool confirm_restart;
} GlobalOptions;

typedef struct LevelSaveData_st
{
    bool completed: 1;
    u8 score: 3;
    u8 time_frames: 6;
    u8 time_seconds: 6;
    u8 time_minutes: 8;
} LevelSaveData;

typedef struct SaveData_st
{
    GlobalOptions opts;
    LevelSaveData completion[SAVE_COMPLETION_ENTRIES_COUNT];
    u16 completed_count;
    u16 checksum;
} SaveData;


// go through all of sram and check status of every slot
enum SaveInitStatus SAVE_init();
enum SaveInitStatus SAVE_init_status();
bool SAVE_is_initialized();

// save the data for the current slot, from the buffer
void SAVE_commit();

// get number of existing save slots
inline u8 SAVE_slot_count() { return SAVE_NUM_SLOTS; }

// select a slot. any unsaved data in current slot buffer will be lost
void SAVE_slot_select(u8 slot_idx);
enum SaveSlotState SAVE_slot_state(u8 slot_idx);

// read and write to/from the slots directly
void SAVE_slot_read(SaveData * dst, u8 slot_idx);
void SAVE_slot_write(SaveData * src, u8 slot_idx);

u8 SAVE_curr_slot();
SaveData * SAVE_curr_data();

const char * SAVE_enum_init_status_str(enum SaveInitStatus st);
const char * SAVE_enum_slot_state_str(enum SaveSlotState st);
