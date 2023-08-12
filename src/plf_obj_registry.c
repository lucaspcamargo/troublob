#include "plf_obj.h"


#define DECL_POBJ_HANDLER(ObjName) extern void PlfObjHandler_##ObjName (PobjData*, void*, enum PobjEventType);

DECL_POBJ_HANDLER(Test)

const PobjEventHandler POBJ_HANDLERS[] =
{
    PlfObjHandler_Test // POBJ_TYPE_TEST,
    // POBJ_TYPE_MIRROR,
    // POBJ_TYPE_LASER,
    // POBJ_TYPE_FAN,
    // POBJ_TYPE_BOMB,
    // POBJ_TYPE_HEAT,
    // POBJ_TYPE_COLD,
    // POBJ_TYPE_PLACE,
    // POBJ_TYPE_COUNT
};
