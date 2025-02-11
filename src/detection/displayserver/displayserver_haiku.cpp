extern "C" {
#include "displayserver.h"
#include "common/settings.h"
#include "common/processing.h"
}

#include <math.h>

#include <Application.h>
#include <Screen.h>

extern "C" void ffConnectDisplayServerImpl(FFDisplayServerResult* ds);

static void detectDisplays(FFDisplayServerResult* ds)
{
    // We need a valid be_app to query the app_server here.
    BApplication app("application/x-vnd.fastfetch-cli-fastfetch");
    BScreen s{}; // default screen is the main one
    bool main = true;

    do
    {
        if (!s.IsValid())
            continue;

        display_mode mode;
        if (s.GetMode(&mode) != B_OK)
            continue;

        FF_STRBUF_AUTO_DESTROY name = ffStrbufCreateA(128);
        monitor_info monitor;
        // WARNING: This is experimental new Haiku API
        status_t err = s.GetMonitorInfo(&monitor);
        if (err == B_OK) {
            ffStrbufSetF(&name, "%s %s", monitor.vendor, monitor.name);
            ffStrbufTrimRightSpace(&name);
        }

        uint32_t width = (uint32_t) s.Frame().Width() + 1;
        uint32_t height = (uint32_t) (uint32_t)s.Frame().Height() + 1;
        double scaleFactor = (double) 1.0;
        ffdsAppendDisplay(ds,
            width,
            height,
            (double)mode.timing.pixel_clock * 1000 / (mode.timing.v_total * mode.timing.h_total),
            (uint32_t) (width / scaleFactor + .5),
            (uint32_t) (height / scaleFactor + .5),
            0,
            0,
            0,
            0,
            &name,
            FF_DISPLAY_TYPE_UNKNOWN,
            main,
            s.ID().id,
            0,
            0,
            "BScreen"
        );
        main = false;
    } while (s.SetToNext() == B_OK);

    return;
}

void ffConnectDisplayServerImpl(FFDisplayServerResult* ds)
{
    ffStrbufSetStatic(&ds->wmProcessName, "app_server");
    ffStrbufSetStatic(&ds->wmPrettyName, "Application Server");
    ffStrbufSetStatic(&ds->dePrettyName, "Application Kit");

    detectDisplays(ds);
}
