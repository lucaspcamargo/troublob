#include "plf_obj.h"
#include "playfield.h"

void PobjHandler_Test(PobjData *data, enum PobjEventType evt, void* evt_arg)
{
    if(evt == POBJ_EVT_CREATED)
    {
        VDP_drawTextBG(BG_B, "OO", fix16ToInt(data->x)*2, fix16ToInt(data->y)*2);
        VDP_drawTextBG(BG_B, "OO", fix16ToInt(data->x)*2, fix16ToInt(data->y)*2 + 1);

        PlfTile *t = (PlfTile*) evt_arg;
        t->attrs |= PLF_ATTR_HOT;
    }
}

