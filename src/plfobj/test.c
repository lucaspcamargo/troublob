#include "plf_obj.h"
#include "playfield.h"
#include <genesis.h>

void PobjHandler_Test(PobjData *data, enum PobjEventType evt, void* evt_arg)
{
    if(evt == POBJ_EVT_CREATED)
    {
        VDP_drawTextEx(BG_B, "OO", TILE_ATTR(0,0,0,0), fix16ToInt(data->x)*2, fix16ToInt(data->y)*2, DMA);
        VDP_drawTextEx(BG_B, "OO", TILE_ATTR(0,0,0,0), fix16ToInt(data->x)*2, fix16ToInt(data->y)*2 + 1, DMA);
    }
}

