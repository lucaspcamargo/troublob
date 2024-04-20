#pragma once

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
    DIREC_CMD_F_NONE = 0x00
} ENUM_PACK;


typedef struct DirectorCommand_st
{
    enum DirectorCommandType type;
    enum DirectorCommandFlags flags;
    u16 arg0;
    u16 arg1;
    void *arg_p;
} DirectorCommand;
