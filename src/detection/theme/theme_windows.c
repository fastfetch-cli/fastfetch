#include "theme.h"

#include "detection/os/os.h"
#include "detection/displayserver/displayserver.h"

const char* ffDetectTheme(FFThemeResult* result) {
    const FFOSResult* os = ffDetectOS();
    const FFDisplayServerResult* displayServer = ffConnectDisplayServer();
    if (ffStrbufEqualS(&displayServer->wmProcessName, "dwm.exe")) {

    uint32_t ver = (uint32_t) ffStrbufToUInt(&os->version, 0);
        if (ver > 1000) {
            // Windows Server
            if (ver >= 2016) {
                ffStrbufSetStatic(&result->theme1, "Fluent");
            } else if (ver >= 2012) {
                ffStrbufSetStatic(&result->theme1, "Metro");
            } else {
                ffStrbufSetStatic(&result->theme1, "Aero");
            }
        } else {
            if (ver >= 10) {
                ffStrbufSetStatic(&result->theme1, "Fluent");
            } else if (ver >= 8) {
                ffStrbufSetStatic(&result->theme1, "Metro");
            } else {
                ffStrbufSetStatic(&result->theme1, "Aero");
            }
        }
    } else if (ffStrbufEqualS(&displayServer->wmProcessName, "explorer.exe")) {
        ffStrbufSetStatic(&result->theme1, "Basic");
    }
    return nullptr;
}
