#pragma once

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// `memrchr` is a GNU extension and may not be declared by system headers even when the symbol exists.
// Declare it unconditionally; the build system provides a fallback implementation when missing.
void* memrchr(const void* s, int c, size_t n);

#ifdef __cplusplus
}
#endif
