#include "memory.h"

#include <windows.h>

const char* ffDetectMemory(FFMemoryResult* ram)
{
    MEMORYSTATUSEX statex = {
        .dwLength = sizeof(statex),
    };
    if (!GlobalMemoryStatusEx(&statex))
        return "GlobalMemoryStatusEx() failed";

    ram->bytesTotal = statex.ullTotalPhys;
    ram->bytesUsed = statex.ullTotalPhys - statex.ullAvailPhys;
    return NULL;
}
