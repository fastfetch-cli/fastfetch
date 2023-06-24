#include "displayserver.h"

void ffConnectDisplayServerImpl(FFDisplayServerResult* ds)
{
    ffStrbufInit(&ds->wmProcessName);
    ffStrbufInit(&ds->wmPrettyName);
    ffStrbufInit(&ds->wmProtocolName);
    ffStrbufInit(&ds->deProcessName);
    ffStrbufInit(&ds->dePrettyName);
    ffStrbufInit(&ds->deVersion);
    ffListInitA(&ds->displays, sizeof(FFDisplayResult), 0);
}
