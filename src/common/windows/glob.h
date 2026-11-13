#pragma once

#include "common/FFstrbuf.h"

// Expands environment variables and glob patterns in a Windows path.
//
// Environment variables may be written as `%NAME%` for compatibility with existing
// configurations, or as `$NAME` / `${NAME}`. A variable that is not defined is left
// untouched, so paths that legitimately contain a `$` or `%` keep working.
//
// Glob patterns support `*`, `?` and `[...]` character classes. Both `/` and `\` are
// accepted as path separators. If more than one path matches, one is chosen
// pseudo-randomly, mirroring the Unix implementation in io_unix.c.
//
// Returns false if a wildcard pattern matches nothing.
[[gnu::nonnull(1, 2)]] bool ffGlobWindows(const char* in, FFstrbuf* out);
