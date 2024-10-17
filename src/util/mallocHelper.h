#pragma once

#include <stdlib.h>
#include <assert.h>

static inline void ffWrapFree(const void* pPtr)
{
    assert(pPtr);
    if(*(void**)pPtr)
        free(*(void**)pPtr);
}

#define FF_AUTO_FREE __attribute__((__cleanup__(ffWrapFree)))
