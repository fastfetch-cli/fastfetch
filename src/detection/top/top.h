#pragma once

#include "fastfetch.h"
#include "modules/top/option.h"

typedef struct FFTopProcessResult {
    FFstrbuf name;
    uint64_t startTime;
    double cpuPercent;
    uint64_t memBytes;
    uint64_t bytesRead;
    uint64_t bytesWritten;
    uint32_t pid;
} FFTopProcessResult;

typedef struct FFTopProcessSnapshot {
    FFstrbuf name;         // comm
    uint64_t cpuTime;      // user_time + system_time
    uint64_t memBytes;     // RSS
    uint64_t bytesRead;    // Physical storage read
    uint64_t bytesWritten; // Physical storage written
    uint64_t startTime;    // Differs between platforms
    uint32_t pid;
} FFTopProcessSnapshot;

const char* ffDetectTopProcesses(FFTopOptions* options, FFlist* result);
const char* ffTopGetProcessSnapshot(FFlist* snapshots);
