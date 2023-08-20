#include "plf_obj.h"


#define DECL_POBJ_HANDLER(ObjName) extern void PobjHandler_##ObjName (PobjData*, enum PobjEventType, void*);

DECL_POBJ_HANDLER(Test)
DECL_POBJ_HANDLER(Laser)
DECL_POBJ_HANDLER(Goal)

const PobjEventHandler POBJ_HANDLERS[] =
{
    PobjHandler_Test,
    NULL,              // POBJ_TYPE_MIRROR,
    PobjHandler_Laser,
    NULL, // POBJ_TYPE_FAN,
    NULL, // POBJ_TYPE_BOMB,
    NULL, // POBJ_TYPE_HEAT,
    NULL, // POBJ_TYPE_COLD,
    PobjHandler_Goal,
};
