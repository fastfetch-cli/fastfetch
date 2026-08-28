#pragma once

#include "fastfetch.h"

typedef struct FFProcessesResult
{
    uint32_t processes;
    uint32_t threads;
} FFProcessesResult;

const char* ffDetectProcesses(FFProcessesResult* result);
