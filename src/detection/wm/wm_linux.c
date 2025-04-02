#include "wm.h"

#include "common/processing.h"
#include "common/io/io.h"
#include "detection/displayserver/displayserver.h"
#include "util/binary.h"
#include "util/path.h"
#include "util/stringUtils.h"

const char* ffDetectWMPlugin(FF_MAYBE_UNUSED FFstrbuf* pluginName)
{
    return "Not supported on this platform";
}

static bool extractHyprlandVersion(const char* line, FF_MAYBE_UNUSED uint32_t len, void *userdata)
{
    int count = 0;
    sscanf(line, "    version: bump to v%*d.%*d.%*d%n", &count);
    if (count == 0) return true;

    // SUPER hacky and doesn't work for development versions
    uint32_t prefixLen = (uint32_t) strlen("    version: bump to v"); // version bump commit message
    ffStrbufSetNS((FFstrbuf*) userdata, len - prefixLen, line + prefixLen);
    return false;
}

static const char* getHyprland(FFstrbuf* result)
{
    FF_STRBUF_AUTO_DESTROY path = ffStrbufCreate();
    const char* error = ffFindExecutableInPath("Hyprland", &path);
    if (error) return "Failed to find Hyprland executable path";

    ffBinaryExtractStrings(path.chars, extractHyprlandVersion, result, (uint32_t) strlen("    version: bump to v0.0.0"));
    if (result->length > 0) return NULL;

    if (ffProcessAppendStdOut(result, (char* const[]){
        path.chars,
        "--version",
        NULL
    }) == NULL)
    { // Hyprland 0.46.2 built from branch v0.46.2-b at... long and multi line
        ffStrbufSubstrAfterFirstC(result, ' ');
        ffStrbufSubstrBeforeFirstC(result, ' ');
        return NULL;
    }

    return "Failed to run command `Hyprland --version`";
}

static bool extractSwayVersion(const char* line, FF_MAYBE_UNUSED uint32_t len, void *userdata)
{
    if (!ffStrStartsWith(line, "sway version ")) return true;

    FFstrbuf* result = (FFstrbuf*) userdata;
    ffStrbufSetNS(result, len - (uint32_t) strlen("sway version "), line + strlen("sway version "));
    ffStrbufTrimRightSpace(result);
    return false;
}

static const char* getSway(FFstrbuf* result)
{
    FF_STRBUF_AUTO_DESTROY path = ffStrbufCreate();
    const char* error = ffFindExecutableInPath("sway", &path);
    if (error) return "Failed to find sway executable path";

    ffBinaryExtractStrings(path.chars, extractSwayVersion, result, (uint32_t) strlen("v0.0.0"));
    if (result->length > 0) return NULL;

    if (ffProcessAppendStdOut(result, (char* const[]){
        path.chars,
        "--version",
        NULL
    }) == NULL)
    { // sway version 1.10
        ffStrbufSubstrAfterLastC(result, ' ');
        ffStrbufTrimRightSpace(result);
        return NULL;
    }

    return "Failed to run command `sway --version`";
}

static const char* getWslg(FFstrbuf* result)
{
    if (!ffAppendFileBuffer("/mnt/wslg/versions.txt", result))
        return "Failed to read /mnt/wslg/versions.txt";

    if (!ffStrbufStartsWithS(result, "WSLg "))
        return "Failed to find WSLg version";

    ffStrbufSubstrBeforeFirstC(result, '\n');
    ffStrbufSubstrBeforeFirstC(result, '+');
    ffStrbufSubstrAfterFirstC(result, ':');
    ffStrbufTrimLeft(result, ' ');
    return NULL;
}

const char* ffDetectWMVersion(const FFstrbuf* wmName, FFstrbuf* result, FF_MAYBE_UNUSED FFWMOptions* options)
{
    if (!wmName)
        return "No WM detected";

    if (ffStrbufEqualS(wmName, "Hyprland"))
        return getHyprland(result);

    if (ffStrbufEqualS(wmName, "sway"))
        return getSway(result);

    if (ffStrbufEqualS(wmName, "WSLg"))
        return getWslg(result);

    return "Unsupported WM";
}
