#pragma once

#include "fastfetch.h"
#include "modules/processes/option.h"

typedef struct FFProcessesResult
{
    uint32_t processes;
    uint32_t threads;
} FFProcessesResult;

const char* ffDetectProcesses(const FFProcessesOptions* options, FFProcessesResult* result);
