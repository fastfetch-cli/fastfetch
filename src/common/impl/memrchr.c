#include "common/memrchr.h"
#include <stddef.h>
#include <stdint.h>

void* memrchr(const void* s, int c, size_t n)
{
    if (n == 0) return NULL;

    const uint8_t uc = (uint8_t) c;

    const uint8_t* p = (const uint8_t*) s + n;

    while (n--)
    {
        if (*--p == uc) return (void*) p;
    }

    return NULL;
}
