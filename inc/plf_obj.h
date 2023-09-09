#pragma once
#include <genesis.h>
#include "dweep_config.h"

// Stuff to handle objects on the playfield


typedef void* PobjHnd;
static const PobjHnd POBJ_HND_INVAL = NULL;

enum PobjType {
    POBJ_TYPE_TEST,
    POBJ_TYPE_MIRROR,
    POBJ_TYPE_LASER,
    POBJ_TYPE_FAN,
    POBJ_TYPE_BOMB,
    POBJ_TYPE_HEAT,
    POBJ_TYPE_COLD,
    POBJ_TYPE_GOAL,
    POBJ_TYPE_TOOL_ITEM,
    POBJ_TYPE_COUNT
} ENUM_PACK;

typedef struct PobjData_st {
    u16 type;
    fix16 x, y;
    char extra[6] __attribute__(( aligned(2) ));
} PobjData;


enum PobjEventType {
    POBJ_EVT_CREATED,     // object is first instantiated
    POBJ_EVT_DESTROYED,   // object data will be deallocated
    POBJ_EVT_STEPPED,     // player is on top of object
    POBJ_EVT_DAMAGE,      // the object suffers damage of some kind (laser hit it, explosion, hammer...)
    POBJ_EVT_LASER_QUERY, // ask about what object does to a laser beam
    POBJ_EVT_TOOL_QUERY,  // get to know which tools can be used on the object
    POBJ_EVT_TOOL,        // tool was used on the object
    POBJ_EVT_FRAME        // invoked on the object every frame (TODO? use bit in handle to not call every obj every frame?)
} ENUM_PACK;

enum PobjLaserBehavior
{
    POBJ_LASER_PASS = 0,
    POBJ_LASER_REFLECT_RIGHT_UP = 1,
    POBJ_LASER_REFLECT_RIGHT_DOWN = 2,
    POBJ_LASER_BLOCK = 3
} ENUM_PACK;

typedef struct PobjEvtCreatedArgs_st {
    void * plftile;
    u16 subtype;
} PobjEvtCreatedArgs;


typedef void (*PobjEventHandler)(PobjData *data, enum PobjEventType evt, void* evt_arg);
extern const PobjEventHandler POBJ_HANDLERS[];


void Pobj_init();    // initialize playfield object system
void Pobj_destroy(); // free playfield object system

PobjHnd Pobj_alloc();                                                           // allocate
void Pobj_dealloc(PobjHnd *handle);                                                // deallocate
inline PobjData * Pobj_get_data(PobjHnd handle) {return (PobjData*) handle;}    // since handle is just pointer to data, optimize this getter here
void Pobj_event(PobjHnd handle, enum PobjEventType evt, void* evt_arg);         // post an event to an object
void Pobj_event_to_all(enum PobjEventType evt, void* evt_arg);                  // post an event to all objects
