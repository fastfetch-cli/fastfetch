#include "os.h"

void ffDetectOSImpl(FFOSResult* os)
{
    ffStrbufSetStatic(&os->name, "NetBSD");
    ffStrbufSet(&os->version, &instance.state.platform.sysinfo.release);
}
