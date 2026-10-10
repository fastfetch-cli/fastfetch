#pragma once

#include "fastfetch.h"

// Returns false when nothing was appended (e.g. `mhz == 0`); callers routinely discard that, so it
// is not `nodiscard`.
[[gnu::nonnull(2)]] bool ffFreqAppendNum(uint32_t mhz, FFstrbuf* result);
