#include "fastfetch.h"

#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <pthread.h>

typedef enum ProtocolHint
{
    FF_PROTOCOL_HINT_UNKNOWN = 0,
    FF_PROTOCOL_HINT_X11,
    FF_PROTOCOL_HINT_WAYLAND
} ProtocolHint;

typedef enum DEHint
{
    FF_DE_HINT_UNKNOWN = 0,
    FF_DE_HINT_PLASMA,
    FF_DE_HINT_GNOME
} DEHint;

typedef struct ProcData
{
    DIR* proc;
    struct dirent* dirent;
    ProtocolHint protocolHint;
    DEHint deHint;
} ProcData;

static inline void getSessionDesktop(FFWMDEResult* result)
{

    result->sessionDesktop = getenv("XDG_CURRENT_DESKTOP");
    if(result->sessionDesktop != NULL && result->sessionDesktop[0] != '\0')
        return;

    result->sessionDesktop = getenv("XDG_SESSION_DESKTOP");
    if(result->sessionDesktop != NULL && result->sessionDesktop[0] != '\0')
        return;

    result->sessionDesktop = getenv("CURRENT_DESKTOP");
    if(result->sessionDesktop != NULL && result->sessionDesktop[0] != '\0')
        return;

    result->sessionDesktop = getenv("SESSION_DESKTOP");
    if(result->sessionDesktop != NULL && result->sessionDesktop[0] != '\0')
        return;

    const char* desktopSession = getenv("DESKTOP_SESSION");
    if(desktopSession != NULL && desktopSession[0] != '\0')
    {
        if(strcasecmp(desktopSession, "plasma") == 0)
            result->sessionDesktop = "KDE";
        else
            result->sessionDesktop = desktopSession;

        return;
    }

    char* gnomeID = getenv("GNOME_DESKTOP_SESSION_ID");
    if(gnomeID != NULL)
    {
        result->sessionDesktop = "Gnome";
        return;
    }

    char* mateID = getenv("MATE_DESKTOP_SESSION_ID");
    if(mateID != NULL)
    {
        result->sessionDesktop = "Mate";
        return;
    }

    char* tdeID = getenv("TDE_FULL_SESSION");
    if(tdeID != NULL)
    {
        result->sessionDesktop = "Trinity";
        return;
    }
}

static bool applyPrettyNameIfWM(FFWMDEResult* result, const FFstrbuf* processName, ProtocolHint* protocolHint)
{
    if(ffStrbufIgnCaseCompS(processName, "kwin_wayland") == 0)
    {
        ffStrbufSetS(&result->wmPrettyName, "KWin");
        *protocolHint = FF_PROTOCOL_HINT_WAYLAND;
    }
    else if(ffStrbufIgnCaseCompS(processName, "kwin_x11") == 0)
    {
        ffStrbufSetS(&result->wmPrettyName, "KWin");
        *protocolHint = FF_PROTOCOL_HINT_X11;
    }
    else if(ffStrbufIgnCaseCompS(processName, "sway") == 0)
    {
        ffStrbufSetS(&result->wmPrettyName, "Sway");
        *protocolHint = FF_PROTOCOL_HINT_WAYLAND;
    }
    else if(ffStrbufIgnCaseCompS(processName, "weston") == 0)
    {
        ffStrbufSetS(&result->wmPrettyName, "Weston");
        *protocolHint = FF_PROTOCOL_HINT_WAYLAND;
    }
    else if(ffStrbufIgnCaseCompS(processName, "wayfire") == 0)
    {
        ffStrbufSetS(&result->wmPrettyName, "Wayfire");
        *protocolHint = FF_PROTOCOL_HINT_WAYLAND;
    }
    else if(ffStrbufIgnCaseCompS(processName, "openbox") == 0)
    {
        ffStrbufSetS(&result->wmPrettyName, "Openbox");
        *protocolHint = FF_PROTOCOL_HINT_X11;
    }
    else if(ffStrbufIgnCaseCompS(processName, "xfwm4") == 0)
    {
        ffStrbufSetS(&result->wmPrettyName, "XFWM");
        *protocolHint = FF_PROTOCOL_HINT_X11;
    }
    else if(ffStrbufIgnCaseCompS(processName, "gnome-session-binary") == 0)
        ffStrbufSetS(&result->wmPrettyName, "Mutter");
    else if(ffStrbufIgnCaseCompS(processName, "cinnamon") == 0)
        ffStrbufSetS(&result->wmPrettyName, "Muffin");

    if(result->wmPrettyName.length > 0)
    {
        ffStrbufSet(&result->wmProcessName, processName);
        return true;
    }

    return false;
}

static bool applyDEHintIfDE(const FFstrbuf* processName, DEHint* deHint)
{
    if(ffStrbufIgnCaseCompS(processName, "plasmashell") == 0)
        *deHint = FF_DE_HINT_PLASMA;
    else if(ffStrbufIgnCaseCompS(processName, "gnome-shell") == 0)
        *deHint = FF_DE_HINT_GNOME;

    return *deHint != FF_DE_HINT_UNKNOWN;
}

static void getFromProcDir(FFWMDEResult* result, ProcData* procData, bool searchWM)
{
    if(procData->proc == NULL)
        return;

    FFstrbuf procPath;
    ffStrbufInitA(&procPath, 64);
    ffStrbufAppendS(&procPath, "/proc/");

    uint32_t procPathLength = procPath.length;

    FFstrbuf processName;
    ffStrbufInit(&processName);

    while((procData->dirent = readdir(procData->proc)) != NULL)
    {
        if(procData->dirent->d_type != DT_DIR)
            continue;

        ffStrbufAppendS(&procPath, procData->dirent->d_name);
        ffStrbufAppendS(&procPath, "/cmdline");
        ffGetFileContent(procPath.chars, &processName);
        ffStrbufRecalculateLength(&processName);
        ffStrbufSubstrAfterLastC(&processName, '/');
        ffStrbufSubstrBefore(&procPath, procPathLength);

        //If the are searching for WM, we are also always searching for DE. Therefore !searchWM must be last in the condition
        if(applyDEHintIfDE(&processName, &procData->deHint) && !searchWM)
            break;

        //If we have a WM, dont overwrite it. Therefoore searchWM must be first in the condition
        if(searchWM && applyPrettyNameIfWM(result, &processName, &procData->protocolHint))
            break;
    }

    ffStrbufDestroy(&processName);
    ffStrbufDestroy(&procPath);
}

static inline void getWM(FFWMDEResult* result, ProcData* procData)
{
    //If sessionDesktop env is a known WM, set it. Otherwise we might be running a DE, search /proc for known WMs
    ffStrbufSetS(&result->wmProcessName, result->sessionDesktop);
    if(!applyPrettyNameIfWM(result, &result->wmProcessName, &procData->protocolHint))
        getFromProcDir(result, procData, true);

    //Fallback for unknown window managers. This will falsely detect DEs as WMs if their WM is unknown
    if(result->wmPrettyName.length == 0)
    {
        ffStrbufSetS(&result->wmProcessName, result->sessionDesktop);
        ffStrbufSetS(&result->wmPrettyName, result->sessionDesktop);
    }
}

static void getKDE(FFWMDEResult* result)
{
    ffStrbufSetS(&result->deProcessName, "plasmashell");
    ffStrbufSetS(&result->dePrettyName, "KDE Plasma");

    ffParsePropFile("/usr/share/xsessions/plasma.desktop", " X-KDE-PluginInfo-Version=%[^\n]", result->deVersion.chars);
    ffStrbufRecalculateLength(&result->deVersion);
}

static void getGnome(FFWMDEResult* result)
{
    ffStrbufSetS(&result->deProcessName, "gnome-session-binary");
    ffStrbufSetS(&result->dePrettyName, "GNOME");

    ffParsePropFile("/usr/share/gnome-shell/org.gnome.Extensions", " version: '%[^']", result->deVersion.chars);
    ffStrbufRecalculateLength(&result->deVersion);
}

static inline void getDEFromHint(FFWMDEResult* result, ProcData* procData)
{
    getFromProcDir(result, procData, false);

    if(procData->deHint == FF_DE_HINT_UNKNOWN)
        return;

    if(procData->deHint == FF_DE_HINT_PLASMA)
        getKDE(result);
    else if(procData->deHint == FF_DE_HINT_GNOME)
        getGnome(result);
}

static inline void getDE(FFWMDEResult* result, ProcData* procData)
{
    // if sessionDesktop is not set or sessionDesktiop == WM, try finding DE via /proc
    if(
        result->sessionDesktop == NULL ||
        *result->sessionDesktop == '\0' ||
        ffStrbufIgnCaseCompS(&result->wmProcessName, result->sessionDesktop) == 0 ||
        ffStrbufIgnCaseCompS(&result->wmPrettyName, result->sessionDesktop) == 0
    ) {
        getDEFromHint(result, procData);
        return;
    }

    if(strcasecmp(result->sessionDesktop, "KDE") == 0)
        getKDE(result);
    else if(strcasecmp(result->sessionDesktop, "Gnome") == 0 || strcasecmp(result->sessionDesktop, "ubuntu:GNOME") == 0 || strcasecmp(result->sessionDesktop, "ubuntu") == 0)
        getGnome(result);
    else
    {
        ffStrbufSetS(&result->deProcessName, result->sessionDesktop);
        ffStrbufSet(&result->dePrettyName, &result->deProcessName);

        if(ffStrbufStartsWithIgnCaseS(&result->dePrettyName, "X-"))
            ffStrbufSubstrAfter(&result->deProcessName, 2);
    }
}

static void getSessionTypeFallback(FFWMDEResult* result, ProtocolHint protocolHint)
{
    if(protocolHint == FF_PROTOCOL_HINT_WAYLAND)
        ffStrbufSetS(&result->wmProtocolName, "Wayland");
    else if(protocolHint == FF_PROTOCOL_HINT_X11)
        ffStrbufSetS(&result->wmProtocolName, "X11");
    else if(getenv("WAYLAND_DISPLAY") != NULL)
        ffStrbufSetS(&result->wmProtocolName, "Wayland");
}

static inline void getSessionType(FFWMDEResult* result, ProtocolHint protocolHint)
{
    const char* xdgSessionType = getenv("XDG_SESSION_TYPE");

    if (xdgSessionType == NULL)
    {
        getSessionTypeFallback(result, protocolHint);
        return;
    }

    if(strcasecmp(xdgSessionType, "wayland") == 0)
        ffStrbufSetS(&result->wmProtocolName, "Wayland");
    else if(strcasecmp(xdgSessionType, "x11") == 0)
        ffStrbufSetS(&result->wmProtocolName, "X11");
    else if(strcasecmp(xdgSessionType, "tty") == 0)
        ffStrbufSetS(&result->wmProtocolName, "TTY");
    else if(strcasecmp(xdgSessionType, "mir") == 0)
        ffStrbufSetS(&result->wmProtocolName, "Mir");
    else
        ffStrbufSetS(&result->wmProtocolName, xdgSessionType);

    // $XDG_SESSION_TYPE is empty
    if(result->wmProtocolName.length == 0)
        getSessionTypeFallback(result, protocolHint);
}

static inline void getWMDE(FFWMDEResult* result)
{
    ProcData procData;
    procData.proc = opendir("/proc");
    procData.dirent = NULL;
    procData.protocolHint = FF_PROTOCOL_HINT_UNKNOWN;
    procData.deHint = FF_DE_HINT_UNKNOWN;

    getSessionDesktop(result);
    getWM(result, &procData);
    getDE(result, &procData);

    if(result->wmProtocolName.length == 0 && procData.protocolHint != FF_PROTOCOL_HINT_UNKNOWN)
        getSessionTypeFallback(result, procData.protocolHint);

    if(procData.proc != NULL)
        closedir(procData.proc);
}

const FFWMDEResult* ffDetectWMDE(FFinstance* instance)
{
    UNUSED(instance);

    static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
    static FFWMDEResult result;
    static bool init = false;
    pthread_mutex_lock(&mutex);
    if(init)
    {
        pthread_mutex_unlock(&mutex);
        return &result;
    }
    init = true;

    ffStrbufInit(&result.wmProcessName);
    ffStrbufInit(&result.wmPrettyName);
    ffStrbufInit(&result.wmProtocolName);
    ffStrbufInit(&result.deProcessName);
    ffStrbufInit(&result.dePrettyName);
    ffStrbufInit(&result.deVersion);

    getSessionType(&result, FF_PROTOCOL_HINT_UNKNOWN);

    //Don't run anyting when on TTY. This prevents us to catch process from other users in at least that case.
    if(ffStrbufIgnCaseCompS(&result.wmProtocolName, "TTY") != 0)
        getWMDE(&result);

    pthread_mutex_unlock(&mutex);

    return &result;
}
