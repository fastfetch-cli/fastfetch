#pragma once

#ifndef FASTFETCH_INCLUDED_UTIL_MALLOC_HELPER
#define FASTFETCH_INCLUDED_UTIL_MALLOC_HELPER

#include <stdlib.h>
#include <assert.h>

static inline void ffWrapFree(void* pPtr)
{
    assert(pPtr);
    if(*(void**)pPtr)
        free(*(void**)pPtr);
}

#define FF_AUTO_FREE __attribute__((__cleanup__(ffWrapFree)))

#endif
