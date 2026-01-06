#include "wallpaper.h"
#include "common/settings.h"
#include "detection/gtk_qt/gtk_qt.h"

const char* ffDetectWallpaper(FFstrbuf* result)
{
    const FFstrbuf* wallpaper = NULL;
    const FFGTKResult* gtk = ffDetectGTK4();
    if (gtk->wallpaper.length)
        wallpaper = &gtk->wallpaper;
    else
    {
        const FFQtResult* qt = ffDetectQt();
        if (qt->wallpaper.length)
            wallpaper = &qt->wallpaper;
    }

    if (!wallpaper)
        return "Failed to detect the current wallpaper path";

    if (ffStrbufStartsWithS(wallpaper, "file:///"))
        ffStrbufAppendS(result, wallpaper->chars + strlen("file://"));
    else
        ffStrbufAppend(result, wallpaper);
    return NULL;
}
