#pragma once

#ifndef FASTFETCH_INCLUDED_UNUSED
#define FASTFETCH_INCLUDED_UNUSED

static inline void ffUnused(int dummy, ...) { (void) dummy; }
#define FF_UNUSED(...) ffUnused(0, __VA_ARGS__);
#define FF_MAYBE_UNUSED __attribute__ ((__unused__))

#endif
