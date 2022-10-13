#include "uptime.h"

uint64_t ffDetectUptime(const FFinstance* instance)
{
    #if FF_HAVE_SYSINFO_H
        return (uint64_t) instance->state.sysinfo.uptime;
    #else
        FF_UNUSED(instance)
        return 0;
    #endif
}
