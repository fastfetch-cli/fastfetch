#include "displayserver.h"

void ffConnectDisplayServerImpl(FFDisplayServerResult* ds, const FFinstance* instance)
{
    FF_UNUSED(instance);

    ffStrbufInitA(&ds->wmProcessName, 0);
    ffStrbufInitA(&ds->wmPrettyName, 0);
    ffStrbufInitA(&ds->wmProtocolName, 0);
    ffStrbufInitA(&ds->deProcessName, 0);
    ffStrbufInitA(&ds->dePrettyName, 0);
    ffStrbufInitA(&ds->deVersion, 0);
    ffListInitA(&ds->resolutions, sizeof(FFResolutionResult), 0);
}
