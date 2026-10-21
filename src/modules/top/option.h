#pragma once

#include "common/option.h"
#include "common/percent.h"

typedef enum FFTopTypes: uint8_t {
    FF_TOP_TYPE_CPU = 1 << 0,
    FF_TOP_TYPE_MEMORY = 1 << 1,
    FF_TOP_TYPE_DISK_READ = 1 << 2,
    FF_TOP_TYPE_DISK_WRITE = 1 << 3,
    FF_TOP_TYPE_START_TIME = 1 << 4,
    FF_TOP_TYPE_THREADS = 1 << 5,

    FF_TOP_TYPE_DISK = FF_TOP_TYPE_DISK_READ | FF_TOP_TYPE_DISK_WRITE,
} FFTopTypes;

typedef struct FFTopOptions {
    FFModuleArgs moduleArgs;
    FFTopTypes sort;
    FFTopTypes showTypes; // bitfields
    uint32_t nProcesses;
    uint32_t waitTime;
    FFPercentageModuleConfig percent;
    bool compact;
} FFTopOptions;

static_assert(sizeof(FFTopOptions) <= FF_OPTION_MAX_SIZE, "FFTopOptions size exceeds maximum allowed size");
