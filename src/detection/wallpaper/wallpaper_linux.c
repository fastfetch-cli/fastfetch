#include "wallpaper.h"
#include "common/settings.h"
#include "detection/gtk_qt/gtk_qt.h"

const char* ffDetectWallpaper(const FFinstance* instance, FFstrbuf* result)
{
    const FFGTKResult* gtk = ffDetectGTK4(instance);
    if (!gtk->wallpaper.length)
        return "Failed to detect the current wallpaper path";

    if (ffStrbufStartsWithS(&gtk->wallpaper, "file:///"))
        ffStrbufAppendS(result, gtk->wallpaper.chars + strlen("file://"));
    else
        ffStrbufAppend(result, &gtk->wallpaper);

    return NULL;
}
