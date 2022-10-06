#pragma once

#ifndef FF_INCLUDED_detection_disk_disk
#define FF_INCLUDED_detection_disk_disk

#include "fastfetch.h"

typedef struct FFDiskResult
{
    FFstrbuf path;
    uint64_t used;
    uint64_t total;
} FFDiskResult;

const char* ffDiskAutodetectFolders(FFinstance* instance, FFlist* folders);

#endif
