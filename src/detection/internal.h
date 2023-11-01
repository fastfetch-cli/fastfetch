#pragma once

#ifndef FF_INCLUDED_detection_internal
#define FF_INCLUDED_detection_internal

#include "common/thread.h"

#define FF_DETECTION_INTERNAL_GUARD(ResultType, ...) \
    static ResultType result; \
    static bool init = false; \
    if(init) \
        return &result; \
    init = true; \
    __VA_ARGS__; \
    return &result; \

#endif
