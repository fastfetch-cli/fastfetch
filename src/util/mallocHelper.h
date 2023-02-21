#include <stdlib.h>

static inline void ffWrapFree(void* pPtr)
{
    assert(pPtr);
    if(*(void**)pPtr)
        free(*(void**)pPtr);
}

#define FF_AUTO_FREE __attribute__((__cleanup__(ffWrapFree)))
