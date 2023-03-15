#pragma once

#ifdef FF_HAVE_JSONC

#include "fastfetch.h"

#include <json-c/json.h>

bool ffJsonLoadLibrary(const FFinstance* instance);

static inline void wrapJsoncFree(json_object** root)
{
    assert(root);
    if (*root)
        json_object_put(*root);
}

#endif
