#pragma once

#ifndef FF_INCLUDED_detection_internal
#define FF_INCLUDED_detection_internal

#include "pthread.h"

#define FF_DETECTION_INTERNAL_GUARD(ResultType, ...) \
    static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER; \
    static ResultType result; \
    static bool init = false; \
    pthread_mutex_lock(&mutex); \
    if(init) \
    { \
        pthread_mutex_unlock(&mutex); \
        return &result; \
    } \
    init = true; \
    __VA_ARGS__; \
    pthread_mutex_unlock(&mutex); \
    return &result; \

#endif
