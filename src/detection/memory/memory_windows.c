#include "memory.h"

void ffDetectMemory(FFMemoryStorage* ram)
{
    MEMORYSTATUSEX statex = {
        .dwLength = sizeof(statex),
    };
    GlobalMemoryStatusEx(&statex);
    ram->bytesTotal = statex.ullTotalPhys;
    ram->bytesUsed = statex.ullTotalPhys - statex.ullAvailPhys;
}
