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
    FF_DE_HINT_GNOME,
    FF_DE_HINT_CINNAMON,
    FF_DE_HINT_XFCE4,
    FF_DE_HINT_MATE,
    FF_DE_HINT_LXQT
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
        ffStrbufSetS(&result->wmPrettyName, "Xfwm4");
        *protocolHint = FF_PROTOCOL_HINT_X11;
    }
    else if(ffStrbufIgnCaseCompS(processName, "Marco") == 0)
    {
        ffStrbufSetS(&result->wmPrettyName, "Marco");
        *protocolHint = FF_PROTOCOL_HINT_X11;
    }
    else if(ffStrbufIgnCaseCompS(processName, "gnome-session-binary") == 0)
        ffStrbufSetS(&result->wmPrettyName, "Mutter");
    else if(ffStrbufIgnCaseCompS(processName, "cinnamon-session") == 0)
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
    else if(ffStrbufIgnCaseCompS(processName, "cinnamon") == 0)
        *deHint = FF_DE_HINT_CINNAMON;
    else if(ffStrbufIgnCaseCompS(processName, "xfce4-session") == 0)
        *deHint = FF_DE_HINT_XFCE4;
    else if(ffStrbufIgnCaseCompS(processName, "mate-session") == 0)
        *deHint = FF_DE_HINT_MATE;
    else if(ffStrbufIgnCaseCompS(processName, "lxqt-session") == 0)
        *deHint = FF_DE_HINT_LXQT;

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
    ffStrbufInitA(&processName, 256); //Some processes have large command lines (looking at you chrome)

    while((procData->dirent = readdir(procData->proc)) != NULL)
    {
        if(procData->dirent->d_type != DT_DIR || procData->dirent->d_name[0] == '.')
            continue;

        ffStrbufAppendS(&procPath, procData->dirent->d_name);
        ffStrbufAppendS(&procPath, "/cmdline");
        ffGetFileContent(procPath.chars, &processName);
        ffStrbufRecalculateLength(&processName); //Arguments are seperated by a \0 char. We make use of this to extract only the executable path. This is needed if arguments contain the / char
        ffStrbufSubstrAfterLastC(&processName, '/');
        ffStrbufSubstrBefore(&procPath, procPathLength);

        //If the are searching for WM, we are also always searching for DE. Therefore !searchWM must be last in the condition
        if(applyDEHintIfDE(&processName, &procData->deHint) && !searchWM)
            break;

        //If we have a WM, dont overwrite it. Therefore searchWM must be first in the condition
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
    ffParsePropFile("/usr/share/xsessions/plasma.desktop", "X-KDE-PluginInfo-Version =", &result->deVersion);
}

static void getGnome(FFWMDEResult* result)
{
    ffStrbufSetS(&result->deProcessName, "gnome-shell");
    ffStrbufSetS(&result->dePrettyName, "GNOME");
    ffParsePropFile("/usr/share/gnome-shell/org.gnome.Extensions", "version :", &result->deVersion);
}

static void getCinnamon(FFWMDEResult* result)
{
    ffStrbufSetS(&result->deProcessName, "cinnamon");
    ffStrbufSetS(&result->dePrettyName, "Cinnamon");
    ffParsePropFile("/usr/share/applications/cinnamon.desktop", "X-GNOME-Bugzilla-Version =", &result->deVersion);
}

static void getMate(FFinstance* instance, FFWMDEResult* result)
{
    ffStrbufSetS(&result->deProcessName, "mate-session");
    ffStrbufSetS(&result->dePrettyName, "MATE");

    //I parse the file 3 times by purpose, because the properties are not guaranteed to be in order
    ffParsePropFile("/usr/share/mate-about/mate-version.xml", "<platform>", &result->deVersion);
    ffStrbufSubstrBeforeFirstC(&result->deVersion, '<');
    ffStrbufAppendC(&result->deVersion, '.');
    ffParsePropFile("/usr/share/mate-about/mate-version.xml", "<minor>", &result->deVersion);
    ffStrbufSubstrBeforeFirstC(&result->deVersion, '<');
    ffStrbufAppendC(&result->deVersion, '.');
    ffParsePropFile("/usr/share/mate-about/mate-version.xml", "<micro>", &result->deVersion);
    ffStrbufSubstrBeforeFirstC(&result->deVersion, '<');

    if(result->deVersion.length == 0 && instance->config.allowSlowOperations)
    {
        ffProcessAppendStdOut(&result->deVersion, (char* const[]){
            "mate-session",
            "--version",
            NULL
        });

        ffStrbufSubstrAfterFirstC(&result->deVersion, ' ');
        ffStrbufTrim(&result->deVersion, ' ');
    }
}

static void getXFCE4(FFinstance* instance, FFWMDEResult* result)
{
    ffStrbufSetS(&result->deProcessName, "xfce4-session");
    ffStrbufSetS(&result->dePrettyName, "Xfce4");
    ffParsePropFile("/usr/share/gtk-doc/html/libxfce4ui/index.html", "<div><p class=\"releaseinfo\">Version", &result->deVersion);

    if(result->deVersion.length == 0 && instance->config.allowSlowOperations)
    {
        //This is really, really slow. Thank you, XFCE developers
        ffProcessAppendStdOut(&result->deVersion, (char* const[]){
            "xfce4-session",
            "--version",
            NULL
        });

        ffStrbufSubstrBeforeFirstC(&result->deVersion, '(');
        ffStrbufSubstrAfterFirstC(&result->deVersion, ' ');
        ffStrbufTrim(&result->deVersion, ' ');
    }
}

static void getLXQt(FFinstance* instance, FFWMDEResult* result)
{
    ffStrbufSetS(&result->deProcessName, "lxqt-session");
    ffStrbufSetS(&result->dePrettyName, "LXQt");

    if(instance->config.allowSlowOperations)
    {
        //This is really, really, reaaaally slow. Thank you, LXQt developers
        ffProcessAppendStdOut(&result->deVersion, (char* const[]){
            "lxqt-session",
            "-v",
            NULL
        });

        ffStrbufSubstrAfter(&result->deVersion, result->deProcessName.length);
        ffStrbufSubstrBefore(&result->deVersion, 6); //X.XX.X
    }
}

static inline void getDEFromHint(FFinstance* instance, FFWMDEResult* result, ProcData* procData)
{
    getFromProcDir(result, procData, false);

    if(procData->deHint == FF_DE_HINT_UNKNOWN)
        return;

    if(procData->deHint == FF_DE_HINT_PLASMA)
        getKDE(result);
    else if(procData->deHint == FF_DE_HINT_GNOME)
        getGnome(result);
    else if(procData->deHint == FF_DE_HINT_CINNAMON)
        getCinnamon(result);
    else if(procData->deHint == FF_DE_HINT_XFCE4)
        getXFCE4(instance, result);
    else if(procData->deHint == FF_DE_HINT_MATE)
        getMate(instance, result);
    else if(procData->deHint == FF_DE_HINT_LXQT)
        getLXQt(instance, result);
}

static inline void getDE(FFinstance* instance, FFWMDEResult* result, ProcData* procData)
{
    // if sessionDesktop is not set or sessionDesktiop == WM, try finding DE via /proc
    if(
        result->sessionDesktop == NULL ||
        *result->sessionDesktop == '\0' ||
        ffStrbufIgnCaseCompS(&result->wmProcessName, result->sessionDesktop) == 0 ||
        ffStrbufIgnCaseCompS(&result->wmPrettyName, result->sessionDesktop) == 0
    ) {
        getDEFromHint(instance, result, procData);
        return;
    }

    if(strcasecmp(result->sessionDesktop, "KDE") == 0 || strcasecmp(result->sessionDesktop, "plasma") == 0 || strcasecmp(result->sessionDesktop, "plasmashell") == 0)
        getKDE(result);
    else if(strcasecmp(result->sessionDesktop, "Gnome") == 0 || strcasecmp(result->sessionDesktop, "ubuntu:GNOME") == 0 || strcasecmp(result->sessionDesktop, "ubuntu") == 0)
        getGnome(result);
    else if(strcasecmp(result->sessionDesktop, "X-Cinnamon") == 0 || strcasecmp(result->sessionDesktop, "Cinnamon") == 0)
        getCinnamon(result);
    else if(strcasecmp(result->sessionDesktop, "XFCE") == 0 || strcasecmp(result->sessionDesktop, "X-XFCE") == 0 || strcasecmp(result->sessionDesktop, "XFCE4") == 0 || strcasecmp(result->sessionDesktop, "X-XFCE4") == 0)
        getXFCE4(instance, result);
    else if(strcasecmp(result->sessionDesktop, "MATE") == 0 || strcasecmp(result->sessionDesktop, "X-MATE") == 0)
        getMate(instance, result);
    else if(strcasecmp(result->sessionDesktop, "LXQt") == 0 || strcasecmp(result->sessionDesktop, "X-LXQT") == 0)
        getLXQt(instance, result);
    else
    {
        ffStrbufSetS(&result->deProcessName, result->sessionDesktop);
        ffStrbufSet(&result->dePrettyName, &result->deProcessName);

        if(ffStrbufStartsWithIgnCaseS(&result->dePrettyName, "X-"))
            ffStrbufSubstrAfter(&result->dePrettyName, 1);
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

static inline void getWMDE(FFinstance* instance, FFWMDEResult* result)
{
    ProcData procData;
    procData.proc = opendir("/proc");
    procData.dirent = NULL;
    procData.protocolHint = FF_PROTOCOL_HINT_UNKNOWN;
    procData.deHint = FF_DE_HINT_UNKNOWN;

    getSessionDesktop(result);
    getWM(result, &procData);
    getDE(instance, result, &procData);

    if(result->wmProtocolName.length == 0 && procData.protocolHint != FF_PROTOCOL_HINT_UNKNOWN)
        getSessionTypeFallback(result, procData.protocolHint);

    if(procData.proc != NULL)
        closedir(procData.proc);
}

const FFWMDEResult* ffDetectWMDE(FFinstance* instance)
{
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
        getWMDE(instance, &result);

    pthread_mutex_unlock(&mutex);
    return &result;
}
