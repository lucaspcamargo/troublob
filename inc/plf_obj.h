#pragma once

/*
This file is part of Dweep Genesis.

Dweep Genesis is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

Dweep Genesis is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with Foobar. If not, see <https://www.gnu.org/licenses/>.
*/

#include <genesis.h>
#include "dweep_config.h"

// Stuff to handle objects on the playfield


typedef void* PobjHnd;
static const PobjHnd POBJ_HND_INVAL = NULL;

enum PobjType {
    POBJ_TYPE_TEST,
    POBJ_TYPE_MIRROR,
    POBJ_TYPE_HEAT,
    POBJ_TYPE_COLD,
    POBJ_TYPE_TOOL_ITEM,
    POBJ_TYPE_FRAME_UPDATE_WATERMARK, // types equal or larger than this receive frame update events
    POBJ_TYPE_LASER = POBJ_TYPE_FRAME_UPDATE_WATERMARK,
    POBJ_TYPE_FAN,
    POBJ_TYPE_BOMB,
    POBJ_TYPE_GOAL,
    POBJ_TYPE_COUNT
} ENUM_PACK;

typedef struct PobjData_st {
    u16 type;
    fix16 x, y;
    char extra[6] __attribute__(( aligned(2) ));
} PobjData;


enum PobjEventType {
    POBJ_EVT_CREATED,         // object is first instantiated
    POBJ_EVT_DESTROYED,       // object data will be deallocated
    POBJ_EVT_STEPPED,         // player is on top of object
    POBJ_EVT_DAMAGE,          // the object suffers damage of some kind (laser hit it, explosion, hammer...)
    POBJ_EVT_LASER_QUERY,     // ask about what object does to a laser beam
    POBJ_EVT_TOOL_QUERY,      // get to know which tools can be used on the object
    POBJ_EVT_TOOL,            // tool was used on the object
    POBJ_EVT_FRAME,           // invoked on the object every frame (TODO? use bit in handle or separate bitmap to not call every obj every frame?)'
    POBJ_EVT_PLANE_A_EVICT,   // something else (player, laser, keepout) needs plane A cleared, now scram, revert to sprite
    POBJ_EVT_PLANE_A_INVITE   // all blockages for plane A usage have been lifted, you may want to claim it and free a sprite
} ENUM_PACK;

enum PobjLaserBehavior
{
    POBJ_LASER_PASS = 0,
    POBJ_LASER_REFLECT_RIGHT_UP = 1,
    POBJ_LASER_REFLECT_RIGHT_DOWN = 2,
    POBJ_LASER_BLOCK = 3
} ENUM_PACK;

enum PobjDamageType
{
    POBJ_DAMAGE_LASER,
    POBJ_DAMAGE_BOMB,
    POBJ_DAMAGE_HAMMER
} ENUM_PACK;

typedef struct PobjEvtCreatedArgs_st {
    void * plftile;
    u16 subtype;
} PobjEvtCreatedArgs;

typedef struct PobjEvtToolQueryArgs_st {
    u16 tool_id;
    bool out_can_use;
    u8 out_cursor;
} PobjEvtToolQueryArgs;

typedef struct PobjEvtToolArgs_st {
    u8 tool_id;  // FIXME u8 is assumption
} PobjEvtToolArgs;

typedef struct PobjEvtDamageArgs_st {
    enum PobjDamageType damage_type;
} PobjEvtDamageArgs;

typedef void (*PobjEventHandler)(PobjData *data, enum PobjEventType evt, void* evt_arg);
extern const PobjEventHandler POBJ_HANDLERS[];


void Pobj_init();    // initialize playfield object system
void Pobj_destroy(); // free playfield object system

PobjHnd Pobj_alloc();                                                           // allocate
void Pobj_dealloc(PobjHnd *handle);                                                // deallocate
inline PobjData * Pobj_get_data(PobjHnd handle) {return (PobjData*) handle;}    // since handle is just pointer to data, optimize this getter here
void Pobj_event(PobjHnd handle, enum PobjEventType evt, void* evt_arg);         // post an event to an object
void Pobj_event_to_all(enum PobjEventType evt, void* evt_arg, bool frame_update);  // post an event to all objects
