#if defined(__sun) && ! defined(__illumos__)
#include "memrchr.h"

void *memrchr(const void *s, int c, size_t n)
{
    if(n == 0) return NULL;
    const unsigned char *p = (const unsigned char *)s + n;
    while (n--) {
        if (*(--p) == (unsigned char) c)
            return (void*) p;
    }

    return NULL;
}
#endif
