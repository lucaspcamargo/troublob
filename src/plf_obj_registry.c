/*
This file is part of Blob Genesis.

Blob Genesis is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

Blob Genesis is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with Foobar. If not, see <https://www.gnu.org/licenses/>.
*/

#include "plf_obj.h"


#define DECL_POBJ_HANDLER(ObjName) extern void PobjHandler_##ObjName (PobjData*, enum PobjEventType, void*);

DECL_POBJ_HANDLER(Test)
DECL_POBJ_HANDLER(Mirror)
DECL_POBJ_HANDLER(Hot)
DECL_POBJ_HANDLER(Cold)
DECL_POBJ_HANDLER(ToolItem)
DECL_POBJ_HANDLER(Laser)
DECL_POBJ_HANDLER(Bomb)
DECL_POBJ_HANDLER(Goal)

const PobjEventHandler POBJ_HANDLERS[] =
{
    PobjHandler_Test,
    PobjHandler_Mirror,
    PobjHandler_Hot,
    PobjHandler_Cold,
    PobjHandler_ToolItem,
    PobjHandler_Laser,  // ------------- FRAME UPDATE WATERMARK
    NULL,               // POBJ_TYPE_FAN,
    PobjHandler_Bomb,
    PobjHandler_Goal,
};
