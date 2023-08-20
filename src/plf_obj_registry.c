#include "plf_obj.h"


#define DECL_POBJ_HANDLER(ObjName) extern void PobjHandler_##ObjName (PobjData*, enum PobjEventType, void*);

DECL_POBJ_HANDLER(Test)
DECL_POBJ_HANDLER(Laser)

const PobjEventHandler POBJ_HANDLERS[] =
{
    PobjHandler_Test,   // POBJ_TYPE_TEST,
    NULL,              // POBJ_TYPE_MIRROR,
    PobjHandler_Laser,  // POBJ_TYPE_LASER,
    // POBJ_TYPE_FAN,
    // POBJ_TYPE_BOMB,
    // POBJ_TYPE_HEAT,
    // POBJ_TYPE_COLD,
    // POBJ_TYPE_PLACE
};
