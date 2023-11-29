#include "displayserver_linux.h"
#include "common/io/io.h"

void ffConnectDisplayServerImpl(FFDisplayServerResult* ds)
{
    ffStrbufInit(&ds->wmProcessName);
    ffStrbufInit(&ds->wmPrettyName);
    ffStrbufInit(&ds->wmProtocolName);
    ffStrbufInit(&ds->deProcessName);
    ffStrbufInit(&ds->dePrettyName);
    ffListInitA(&ds->displays, sizeof(FFDisplayResult), 4);

    if (!instance.config.general.dsForceDrm)
    {
        //We try wayland as our preferred display server, as it supports the most features.
        //This method can't detect the name of our WM / DE
        ffdsConnectWayland(ds);

        //Try the x11 libs, from most feature rich to least.
        //We use the display list to detect if a connection is needed.
        //They respect wmProtocolName, and only detect display if it is set.

        if(ds->displays.length == 0)
            ffdsConnectXcbRandr(ds);

        if(ds->displays.length == 0)
            ffdsConnectXrandr(ds);

        if(ds->displays.length == 0)
            ffdsConnectXcb(ds);

        if(ds->displays.length == 0)
            ffdsConnectXlib(ds);
    }

    //This display detection method is display server independent.
    //Use it if all connections failed
    if(ds->displays.length == 0)
        ffdsConnectDrm(ds);

    //This fills in missing information about WM / DE by using env vars and iterating processes
    ffdsDetectWMDE(ds);
}
