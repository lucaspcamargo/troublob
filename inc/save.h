#pragma once

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
