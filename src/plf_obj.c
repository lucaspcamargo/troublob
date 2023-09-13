#include <genesis.h>
#include "plf_obj.h"

#define POBJ_MAX 128   // num of max objects that can be allocated
#define POBJ_SZ (sizeof(PobjData))

static Pool * _pobj_pool;

void Pobj_init()
{
    if(_pobj_pool)
        Pobj_destroy();

    _pobj_pool = POOL_create(POBJ_MAX, POBJ_SZ);
}

void Pobj_destroy()
{
    if(!_pobj_pool)
        return;
    POOL_destroy(_pobj_pool);
    _pobj_pool = NULL;
}

PobjHnd Pobj_alloc()
{
    void *ptr = POOL_allocate(_pobj_pool);
    if (ptr)
        return (PobjHnd) ptr;
    else
        return POBJ_HND_INVAL;
}

void Pobj_dealloc(PobjHnd *obj)
{
    if(obj)
        POOL_release(_pobj_pool, obj, TRUE); // TRUE means reorder pointer list (maybe slow), but eases iteration
}

/* No need to convert, pointer is to obj data anyway
PobjData * Pobj_data(PobjHnd handle)
{
(...)
}*/

void Pobj_event(PobjHnd handle, enum PobjEventType evt, void* evt_arg)
{
    PobjEventHandler hnd = POBJ_HANDLERS[((PobjData*)handle)->type];
    if(hnd)
        (*hnd)(handle, evt, evt_arg);
}


void Pobj_event_to_all(enum PobjEventType evt, void* evt_arg, bool frame_update)
{
    void ** base_ptr = POOL_getFirst(_pobj_pool);
    u16 cnt = POOL_getNumAllocated(_pobj_pool);
    for(u16 i = 0; i < cnt; i++)
    {
        if(frame_update && ((PobjData*)base_ptr[i])->type < POBJ_TYPE_FRAME_UPDATE_WATERMARK)
            continue;
        Pobj_event((PobjData*)base_ptr[i], evt, evt_arg);
    }
}
