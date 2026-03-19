#include "uptime.h"
#include "common/time.h"
#include "common/windows/nt.h"

const char* ffDetectUptime(FFUptimeResult* result)
{
    // GetInterruptTime with Win7 support
    uint64_t interruptTime = ffKSystemTimeToUInt64(&SharedUserData->InterruptTime);

    result->uptime = interruptTime / 10000; // Convert from 100-nanosecond intervals to milliseconds
    result->bootTime = ffTimeGetNow() - result->uptime;

    // Alternatively, `NtQuerySystemInformation(SystemTimeOfDayInformation)` reports the boot time directly,
    // whose result exactly equals what WMI `Win32_OperatingSystem` reports
    // with much lower accuracy (0.5 seconds)

    return NULL;
}
