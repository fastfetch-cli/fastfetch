#pragma once

#include "fastfetch.h"

typedef struct FFBtrfsDiskUsage
{
    uint64_t total;
    uint64_t used;
    const char* type;
    bool dup;
} FFBtrfsDiskUsage;

typedef struct FFBtrfsResult
{
    FFstrbuf name;
    FFstrbuf uuid;
    FFstrbuf devices;
    FFstrbuf features;
    uint32_t generation;
    uint32_t nodeSize;
    uint32_t sectorSize;
    uint64_t totalSize;
    uint64_t globalReservationUsed;
    uint64_t globalReservationTotal;
    FFBtrfsDiskUsage allocation[3];
} FFBtrfsResult;

const char* ffDetectBtrfs(FFlist* result /* list of FFBtrfsResult */);
