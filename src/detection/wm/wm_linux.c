#include "wm.h"

#include "common/processing.h"
#include "common/io/io.h"
#include "detection/displayserver/displayserver.h"
#include "util/binary.h"
#include "util/path.h"
#include "util/stringUtils.h"
#include "util/debug.h"

const char* ffDetectWMPlugin(FF_MAYBE_UNUSED FFstrbuf* pluginName)
{
    return "Not supported on this platform";
}

static bool extractHyprlandVersion(const char* line, uint32_t len, void *userdata)
{
    if (line[0] != 'v') return true;
    ++line; --len;
    int count = 0;
    sscanf(line, "%*d.%*d.%*d%n", &count);
    if (count == 0) return true;

    ffStrbufSetNS((FFstrbuf*) userdata, len, line);
    return false;
}

static const char* getHyprland(FFstrbuf* result)
{
    FF_DEBUG("Detecting Hyprland version");
    FF_STRBUF_AUTO_DESTROY buffer = ffStrbufCreate();

    FF_DEBUG("Checking for " FASTFETCH_TARGET_DIR_USR "/include/hyprland/src/version.h" " file");
    if (ffReadFileBuffer(FASTFETCH_TARGET_DIR_USR "/include/hyprland/src/version.h", result))
    {
        FF_DEBUG("Found version.h file, extracting version");
        if (ffStrbufSubstrAfterFirstS(result, "\n#define GIT_TAG "))
        {
            ffStrbufSubstrAfterFirstC(result, '"');
            ffStrbufSubstrBeforeFirstC(result, '"');
            ffStrbufTrimLeft(result, 'v');
            FF_DEBUG("Extracted version from version.h: %s", result->chars);
            return NULL;
        }
        FF_DEBUG("Failed to extract version from version.h");
        ffStrbufClear(result);
    }
    else
    {
        FF_DEBUG("version.h file not found, trying Hyprland executable");
    }

    const char* error = ffFindExecutableInPath("Hyprland", &buffer);
    if (error) {
        FF_DEBUG("Error finding Hyprland executable: %s", error);
        return "Failed to find Hyprland executable path";
    }
    FF_DEBUG("Found Hyprland executable at: %s", buffer.chars);

    ffBinaryExtractStrings(buffer.chars, extractHyprlandVersion, result, (uint32_t) strlen("v0.0.0"));
    if (result->length > 0) {
        FF_DEBUG("Extracted version from binary strings: %s", result->chars);
        return NULL;
    }
    FF_DEBUG("Failed to extract version from binary strings, trying --version option");

    if (ffProcessAppendStdOut(result, (char* const[]){
        buffer.chars,
        "--version",
        NULL
    }) == NULL)
    {
        // Hyprland 0.48.1 built from branch  at commit 29e2e59...
        // Date: ...
        // Tag: v0.48.1, commits: 5937
        // ...

        FF_DEBUG("Raw version output: %s", result->chars);
        // Use tag if available
        if (ffStrbufSubstrAfterFirstS(result, "\nTag: v"))
        {
            ffStrbufSubstrBeforeFirstC(result, ',');
            FF_DEBUG("Extracted version from Tag: %s", result->chars);
        }
        else
        {
            ffStrbufSubstrAfterFirstC(result, ' ');
            ffStrbufSubstrBeforeFirstC(result, ' ');
            FF_DEBUG("Extracted version from output: %s", result->chars);
        }
        return NULL;
    }
    FF_DEBUG("Failed to run Hyprland --version command");

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
