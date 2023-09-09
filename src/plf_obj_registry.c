#include "plf_obj.h"


#define DECL_POBJ_HANDLER(ObjName) extern void PobjHandler_##ObjName (PobjData*, enum PobjEventType, void*);

DECL_POBJ_HANDLER(Test)
DECL_POBJ_HANDLER(Mirror)
DECL_POBJ_HANDLER(Laser)
DECL_POBJ_HANDLER(Hot)
DECL_POBJ_HANDLER(Cold)
DECL_POBJ_HANDLER(Goal)
DECL_POBJ_HANDLER(ToolItem)

const PobjEventHandler POBJ_HANDLERS[] =
{
    PobjHandler_Test,
    PobjHandler_Mirror,
    PobjHandler_Laser,
    NULL, // POBJ_TYPE_FAN,
    NULL, // POBJ_TYPE_BOMB,
    PobjHandler_Hot, // POBJ_TYPE_HEAT,
    PobjHandler_Cold, // POBJ_TYPE_COLD,
    PobjHandler_Goal,
    PobjHandler_ToolItem
};
