#pragma once

static inline void ffUnused(...) { /* no-op */ }
#define FF_UNUSED(...) ffUnused(__VA_ARGS__);
