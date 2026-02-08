#pragma once
#include <stddef.h>

#if defined(__sun) && !defined(__illumos__)
void *memrchr(const void *s, int c, size_t n);
#endif