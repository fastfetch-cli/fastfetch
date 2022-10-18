#pragma once

#ifndef FF_INCLUDED_detection_storage
#define FF_INCLUDED_detection_storage

#include "fastfetch.h"

typedef struct FFMemoryStorage
{
    FFstrbuf error;
    uint64_t bytesUsed;
    uint64_t bytesTotal;
    uint8_t percentage;
} FFMemoryStorage;

#endif
