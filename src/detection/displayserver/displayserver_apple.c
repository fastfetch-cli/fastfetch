#include "displayserver.h"

void ffConnectDisplayServerImpl(FFDisplayServerResult* ds, const FFinstance* instance)
{
    FF_UNUSED(instance);

    ffStrbufInit(&ds->wmProcessName);
    ffStrbufAppendS(&ds->wmProcessName, "quartz");

    ffStrbufInit(&ds->wmPrettyName);
    ffStrbufAppendS(&ds->wmPrettyName, "Quartz Compositor");

    ffStrbufInit(&ds->deProcessName);
    ffStrbufAppendS(&ds->deProcessName, "aqua");

    ffStrbufInit(&ds->dePrettyName);
    ffStrbufAppendS(&ds->dePrettyName, "Aqua");

    ffStrbufInitA(&ds->deVersion, 0);
    ffStrbufInitA(&ds->wmProtocolName, 0);
    ffListInitA(&ds->resolutions, sizeof(FFResolutionResult), 0);
}
