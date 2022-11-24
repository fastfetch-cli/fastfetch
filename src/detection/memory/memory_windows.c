#include "memory.h"

void ffDetectMemoryImpl(FFMemoryStorage* ram)
{
    MEMORYSTATUSEX statex = {
        .dwLength = sizeof(statex),
    };
    GlobalMemoryStatusEx(&statex);
    ram->bytesTotal = statex.ullTotalPhys;
    ram->bytesUsed = statex.ullTotalPhys - statex.ullAvailPhys;
}
