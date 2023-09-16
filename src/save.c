#include "save.h"

void SAVE_init()
{
    SRAM_enableRO();

    SRAM_disable();
}
