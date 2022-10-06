#include "processes.h"

uint32_t ffDetectProcesses(FFinstance* instance, FFstrbuf* error)
{
    #if FF_HAVE_SYSINFO_H
        FF_UNUSED(error);
        return (uint32_t) instance->state.sysinfo.procs;
    #else
        ffStrbufAppendS(error, "Unimplemented");
        return 0;
    #endif
}
