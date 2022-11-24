#include "swap.h"

void ffDetectSwapImpl(FFMemoryStorage* swap)
{
    MEMORYSTATUSEX statex = {
        .dwLength = sizeof(statex),
    };
    GlobalMemoryStatusEx(&statex);
    swap->bytesTotal = statex.ullTotalPageFile;
    swap->bytesUsed = statex.ullTotalPageFile - statex.ullAvailPageFile;
}
