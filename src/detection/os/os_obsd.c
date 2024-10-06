#include "os.h"

void ffDetectOSImpl(FFOSResult* os)
{
    ffStrbufSetStatic(&os->name, "OpenBSD");
    ffStrbufSet(&os->version, &instance.state.platform.sysinfo.release);
}
