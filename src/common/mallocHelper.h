#pragma once

#include <stdlib.h>
#include <assert.h>

#if FF_HAVE_MALLOC_USABLE_SIZE || FF_HAVE_MSVC_MSIZE
    #if __has_include(<malloc.h>)
        #include <malloc.h>
    #else
        #include <malloc_np.h> // For DragonFly BSD
    #endif
#elif FF_HAVE_MALLOC_SIZE
    #include <malloc/malloc.h>
#endif

static inline void ffWrapFree(const void* pPtr)
{
    assert(pPtr);
    if(*(void**)pPtr)
        free(*(void**)pPtr);
}

#define FF_AUTO_FREE __attribute__((__cleanup__(ffWrapFree)))

// ptr MUST be a malloc'ed pointer
static inline size_t ffMallocUsableSize(const void* ptr)
{
    assert(ptr);
    #if FF_HAVE_MALLOC_USABLE_SIZE
        return malloc_usable_size((void*) ptr);
    #elif FF_HAVE_MALLOC_SIZE
        return malloc_size((void*) ptr);
    #elif FF_HAVE_MSVC_MSIZE
        return _msize((void*) ptr);
    #else
        (void) ptr;
        return 0; // Not supported
    #endif
}
