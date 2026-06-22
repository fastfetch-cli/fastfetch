#include "wallpaper.h"
#include "common/io.h"
#include "common/settings.h"
#include "detection/displayserver/displayserver.h"
#include "detection/gtk_qt/gtk_qt.h"

static const char* detectCosmicComp(FFstrbuf* result) {
    FF_STRBUF_AUTO_DESTROY path = ffStrbufCreateCopy(FF_LIST_FIRST(FFstrbuf, instance.state.platform.configDirs));
    ffStrbufAppendS(&path, "cosmic/com.system76.CosmicBackground/v1/");
    uint32_t basePathLength = path.length;

    FF_STRBUF_AUTO_DESTROY backgrounds = ffStrbufCreate();
    ffStrbufAppendS(&path, "backgrounds");
    bool sharedWallpaper = true;
    FF_STRBUF_AUTO_DESTROY monitorName = ffStrbufCreate();
    if (ffReadFileBuffer(path.chars, &backgrounds)) {
        FF_STRBUF_AUTO_DESTROY cleaned = ffStrbufCreateCopy(&backgrounds);
        ffStrbufRemoveStrings(&cleaned, 4, (const char*[]){ " ", "\n", "\r", "\t" });

        if (!ffStrbufEqualS(&cleaned, "[]")) {
            const char* firstQuote = strchr(backgrounds.chars, '"');
            if (firstQuote != NULL) {
                const char* secondQuote = strchr(firstQuote + 1, '"');
                if (secondQuote != NULL && secondQuote > firstQuote + 1) {
                    ffStrbufAppendNS(&monitorName, (uint32_t) (secondQuote - firstQuote - 1), firstQuote + 1);
                    sharedWallpaper = false;
                }
            }
        }
    }
    ffStrbufSubstrBefore(&path, basePathLength);

    if (sharedWallpaper) {
        ffStrbufAppendS(&path, "all");
    } else {
        ffStrbufAppendS(&path, "output.");
        ffStrbufAppend(&path, &monitorName);
    }

    FF_STRBUF_AUTO_DESTROY output = ffStrbufCreate();
    if (!ffReadFileBuffer(path.chars, &output)) {
        return "Failed to read COSMIC wallpaper config";
    }

    const char* sourceStart = strstr(output.chars, "source:");
    if (sourceStart == NULL) {
        return "COSMIC wallpaper config doesn't contain source";
    }

    const char* pathStart = strstr(sourceStart, "Path(");
    if (pathStart == NULL) {
        return "COSMIC wallpaper source is not a Path value";
    }

    pathStart += strlen("Path(");
    while (*pathStart == ' ' || *pathStart == '\t') {
        ++pathStart;
    }

    if (*pathStart != '\'' && *pathStart != '"') {
        return "COSMIC wallpaper Path format is invalid";
    }

    char quote = *pathStart;
    ++pathStart;
    const char* pathEnd = strchr(pathStart, quote);
    if (pathEnd == NULL || pathEnd == pathStart) {
        return "COSMIC wallpaper path is empty";
    }

    ffStrbufAppendNS(result, (uint32_t) (pathEnd - pathStart), pathStart);
    return NULL;
}

const char* ffDetectWallpaper(FFstrbuf* result) {
    const FFDisplayServerResult* wm = ffConnectDisplayServer();
    if (ffStrbufIgnCaseEqualS(&wm->wmPrettyName, FF_WM_PRETTY_COSMIC_COMP)) {
        return detectCosmicComp(result);
    }

    const FFstrbuf* wallpaper = NULL;
    const FFGTKResult* gtk = ffDetectGTK4();
    if (gtk->wallpaper.length) {
        wallpaper = &gtk->wallpaper;
    } else {
        const FFQtResult* qt = ffDetectQt();
        if (qt->wallpaper.length) {
            wallpaper = &qt->wallpaper;
        }
    }

    if (!wallpaper) {
        return "Failed to detect the current wallpaper path";
    }

    if (ffStrbufStartsWithS(wallpaper, "file:///")) {
        ffStrbufAppendS(result, wallpaper->chars + strlen("file://"));
    } else {
        ffStrbufAppend(result, wallpaper);
    }
    return NULL;
}
