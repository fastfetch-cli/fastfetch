#pragma once

#ifndef FF_INCLUDED_detection_internal
#define FF_INCLUDED_detection_internal

#include "common/thread.h"

#define FF_DETECTION_INTERNAL_GUARD(ResultType, ...) \
    static FFThreadMutex mutex = FF_THREAD_MUTEX_INITIALIZER; \
    static ResultType result; \
    static bool init = false; \
    ffThreadMutexLock(&mutex); \
    if(init) \
    { \
        ffThreadMutexUnlock(&mutex); \
        return &result; \
    } \
    init = true; \
    __VA_ARGS__; \
    ffThreadMutexUnlock(&mutex); \
    return &result; \

#endif
