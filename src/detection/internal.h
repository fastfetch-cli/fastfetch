#pragma once

#include "common/thread.h"

#define FF_DETECTION_INTERNAL_GUARD(ResultType, ...) \
    static ResultType result; \
    static bool init = false; \
    if(init) \
        return &result; \
    init = true; \
    __VA_ARGS__; \
    return &result;
