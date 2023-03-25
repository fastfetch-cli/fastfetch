#include "memory.h"

#include <windows.h>

void ffDetectMemory(FFMemoryStorage* ram)
{
    MEMORYSTATUSEX statex = {
        .dwLength = sizeof(statex),
    };
    if (!GlobalMemoryStatusEx(&statex))
    {
        ffStrbufAppendS(&ram->error, "GlobalMemoryStatusEx() failed");
        return;
    }
    ram->bytesTotal = statex.ullTotalPhys;
    ram->bytesUsed = statex.ullTotalPhys - statex.ullAvailPhys;
}
