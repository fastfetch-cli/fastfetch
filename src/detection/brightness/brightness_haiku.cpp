extern "C" {
#include "brightness.h"
#include "common/strutil.h"
}

#include <Application.h>
#include <Screen.h>

const char* ffDetectBrightness(FF_A_UNUSED FFBrightnessOptions* options, FFlist* result) {
    // We need a valid be_app to query the app_server here.
    BApplication app("application/x-vnd.fastfetch-cli-fastfetch");
    BScreen screen{}; // default screen is the main one

    do {
        if (!screen.IsValid()) {
            continue;
        }

        float value = 1.0f;
        status_t status = screen.GetBrightness(&value);
        if (status != B_OK) {
            continue;
        }

        monitor_info monitor;
        // WARNING: This is experimental new Haiku API
        status_t err = screen.GetMonitorInfo(&monitor);

        FFBrightnessResult* brightness = FF_LIST_ADD(FFBrightnessResult, *result);

        if (err == B_OK) {
            ffStrbufInitF(&brightness->name, "%s %s (%ld)", monitor.vendor, monitor.name, screen.ID().id);
            ffStrbufTrimRightSpace(&brightness->name);
        } else {
            ffStrbufInitF(&brightness->name, "Screen %ld", screen.ID().id);
        }

        brightness->max = 1.0f;
        brightness->min = 0.0f;
        brightness->current = value;
        brightness->builtin = true;

    } while (screen.SetToNext() == B_OK);

    return NULL;
}
