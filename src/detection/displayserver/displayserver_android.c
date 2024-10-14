#include "displayserver.h"
#include "common/settings.h"
#include "common/processing.h"

#include <math.h>

static void detectWithDumpsys(FFDisplayServerResult* ds)
{
    FF_STRBUF_AUTO_DESTROY buf = ffStrbufCreate();
    if (ffProcessAppendStdOut(&buf, (char* []) {
        "/system/bin/dumpsys",
        "display",
        NULL,
    }) != NULL || buf.length == 0)
        return; // Only works in `adb shell`, or when rooted

    uint32_t index = 0;
    while ((index = ffStrbufNextIndexS(&buf, index, "DisplayDeviceInfo")) < buf.length)
    {
        index += strlen("DisplayDeviceInfo");
        uint32_t nextIndex = ffStrbufNextIndexC(&buf, index, '\n');
        buf.chars[nextIndex] = '\0';
        const char* info = buf.chars + index;

        // {"Builtin display": uniqueId="local:4630947134992368259", 1440 x 3200, modeId 2, defaultModeId 1, supportedModes [{id=1, width=1440, height=3200, fps=60.000004, alternativeRefreshRates=[24.000002, 30.000002, 40.0, 120.00001, 120.00001, 120.00001, 120.00001, 120.00001]},
        FF_STRBUF_AUTO_DESTROY name = ffStrbufCreateA(64);
        unsigned width = 0, height = 0, modeId = 0;
        double refreshRate = 0;
        // {"Builtin display": uniqueId="local:4630947134992368259", 1440 x 3200, modeId 2
        int res = sscanf(info, "{\"%63[^\"]\":%*s%u x %u, modeId%u", name.chars, &width, &height, &modeId);
        if (res >= 3)
        {
            if (res == 4)
            {
                ++info; // skip first '{'
                while ((info = strchr(info, '{')))
                {
                    ++info;

                    unsigned id;
                    double fps;
                    // id=1, width=1440, height=3200, fps=60.000004,
                    if (sscanf(info, "id=%u, %*s%*s fps=%lf", &id, &fps) >= 2)
                    {
                        if (id == modeId)
                        {
                            refreshRate = fps;
                            break;
                        }
                    }
                    else
                        break;
                }
            }

            ffStrbufRecalculateLength(&name);
            ffdsAppendDisplay(ds,
                (uint32_t)width,
                (uint32_t)height,
                refreshRate,
                0,
                0,
                0,
                &name,
                FF_DISPLAY_TYPE_UNKNOWN,
                false,
                0,
                0,
                0,
                "dumpsys"
            );
        }

        index = nextIndex + 1;
    }
}

static bool detectWithGetprop(FFDisplayServerResult* ds)
{
    // Only for MiUI
    FF_STRBUF_AUTO_DESTROY buffer = ffStrbufCreate();

    if (ffSettingsGetAndroidProperty("persist.sys.miui_resolution", &buffer) &&
        ffStrbufContainC(&buffer, ','))
    {
        // 1440,3200,560 => width,height,densityDpi
        uint32_t width = (uint32_t) ffStrbufToUInt(&buffer, 0);
        ffStrbufSubstrAfterFirstC(&buffer, ',');
        uint32_t height = (uint32_t) ffStrbufToUInt(&buffer, 0);
        ffStrbufSubstrAfterFirstC(&buffer, ',');
        double scaleFactor = (double) ffStrbufToUInt(&buffer, 0) / 160.;
        return ffdsAppendDisplay(ds,
            width,
            height,
            0,
            (uint32_t) (width / scaleFactor + .5),
            (uint32_t) (height / scaleFactor + .5),
            0,
            0,
            FF_DISPLAY_TYPE_BUILTIN,
            false,
            0,
            0,
            0,
            "getprop"
        );
    }

    return false;
}

void ffConnectDisplayServerImpl(FFDisplayServerResult* ds)
{
    ffStrbufSetStatic(&ds->wmProcessName, "WindowManager");
    ffStrbufSetStatic(&ds->wmPrettyName, "Window Manager");

    if (!detectWithGetprop(ds))
        detectWithDumpsys(ds);
}
