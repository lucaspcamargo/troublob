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

#define SAVE_COMPLETION_ENTRIES_COUNT 32
#define SAVE_CHECKSUM_SALT 0x9395

typedef struct SaveData_st
{
    bool completion[SAVE_COMPLETION_ENTRIES_COUNT];
    u16 checksum;
} SaveData;

void SAVE_init();
