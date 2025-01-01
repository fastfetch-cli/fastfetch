#include "wm.h"

#include "common/processing.h"
#include "detection/displayserver/displayserver.h"

const char* ffDetectWMPlugin(FF_MAYBE_UNUSED FFstrbuf* pluginName)
{
    return "Not supported on this platform";
}

static void getHyprland(FFstrbuf* result, FF_MAYBE_UNUSED FFWMOptions* options)
{
    if (ffProcessAppendStdOut(result, (char* const[]){
        "Hyprland",
        "--version",
        NULL
    }) == NULL){ // Hyprland 0.46.2 built from branch v0.46.2-b at... long and multi line
        ffStrbufSubstrAfterFirstC(result, ' ');
        ffStrbufSubstrBeforeFirstC(result, ' ');
    }
}

const char* ffDetectWMVersion(const FFstrbuf* wmName, FFstrbuf* result, FFWMOptions* options)
{
    if (ffStrbufEqualS(wmName, FF_WM_PRETTY_HYPRLAND))
        getHyprland(result, options);
    else
        return "Unsupported WM";
    return NULL;
}
