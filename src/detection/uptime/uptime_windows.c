#include "uptime.h"
#include "common/time.h"
#include "common/library.h"

#include <realtimeapiset.h>
#include <sysinfoapi.h>

const char* ffDetectUptime(FFUptimeResult* result)
{
    #if FF_WIN7_COMPAT
    HMODULE hKernelBase = GetModuleHandleW(L"KernelBase.dll");
    if (__builtin_expect(!!hKernelBase, true))
    {
        FF_LIBRARY_LOAD_SYMBOL_LAZY(hKernelBase, QueryInterruptTime);
        if (ffQueryInterruptTime) // Windows 10 and later
        {
            uint64_t uptime;
            ffQueryInterruptTime(&uptime);
            result->uptime = uptime / 10000; // Convert from 100-nanosecond intervals to milliseconds
            goto ok;
        }
    }

    result->uptime = GetTickCount64();
ok:

    #else
    uint64_t uptime;
    QueryInterruptTime(&uptime);
    result->uptime = uptime / 10000; // Convert from 100-nanosecond intervals to milliseconds
    #endif
    result->bootTime = ffTimeGetNow() - result->uptime;
    return NULL;
}
