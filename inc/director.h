#pragma once

/*
This file is part of Dweep Genesis.

Dweep Genesis is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

Dweep Genesis is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with Foobar. If not, see <https://www.gnu.org/licenses/>.
*/


#include "dweep_config.h"
#include <genesis.h>

enum DirectorCommandType
{
    DIREC_CMD_DEBUG_MENU,
    DIREC_CMD_TITLE,
    DIREC_CMD_LEVEL,
    DIREC_CMD_INVAL
} ENUM_PACK;

enum DirectorCommandFlags
{
    DIREC_CMD_F_NONE = 0x00,
    DIREC_CMD_F_MENU = 0x01,
} ENUM_PACK;


typedef struct DirectorCommand_st
{
    enum DirectorCommandType type;
    enum DirectorCommandFlags flags;
    u16 arg0;
    u16 arg1;
    void *arg_p;
} DirectorCommand;
