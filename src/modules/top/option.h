#pragma once

#include "common/option.h"
#include "common/percent.h"

typedef enum FFTopSortType {
    FF_TOP_SORT_CPU = 0,
    FF_TOP_SORT_MEMORY,
    FF_TOP_SORT_DISK_READ,
    FF_TOP_SORT_DISK_WRITE,
} FFTopSortType;

typedef struct FFTopOptions {
    FFModuleArgs moduleArgs;
    FFTopSortType sort;
    uint32_t nProcesses;
    uint32_t waitTime;
    FFPercentageModuleConfig percent;
    bool compact;
    bool kernelTask; // Whether to include kernel tasks (kworker, System, kernel_task) in the process list
} FFTopOptions;

static_assert(sizeof(FFTopOptions) <= FF_OPTION_MAX_SIZE, "FFTopOptions size exceeds maximum allowed size");
