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

static void getSessionDesktop(FFWMDEResult* result)
{
    result->sessionDesktop = getenv("XDG_CURRENT_DESKTOP");
    if(result->sessionDesktop != NULL && *result->sessionDesktop != '\0')
        return;

    result->sessionDesktop = getenv("XDG_SESSION_DESKTOP");
    if(result->sessionDesktop != NULL && *result->sessionDesktop != '\0')
        return;

    result->sessionDesktop = getenv("CURRENT_DESKTOP");
    if(result->sessionDesktop != NULL && *result->sessionDesktop != '\0')
        return;

    result->sessionDesktop = getenv("SESSION_DESKTOP");
    if(result->sessionDesktop != NULL && *result->sessionDesktop != '\0')
        return;

    char* env;

    env = getenv("DESKTOP_SESSION");
    if(env != NULL && *env != '\0')
    {
        if(strcasecmp(env, "plasma") == 0)
            result->sessionDesktop = "KDE";
        else
            result->sessionDesktop = env;
        return;
    }

    env = getenv("GNOME_DESKTOP_SESSION_ID");
    if(env != NULL)
    {
        result->sessionDesktop = "Gnome";
        return;
    }

    env = getenv("MATE_DESKTOP_SESSION_ID");
    if(env != NULL)
    {
        result->sessionDesktop = "Mate";
        return;
    }

    env = getenv("TDE_FULL_SESSION");
    if(env != NULL)
    {
        result->sessionDesktop = "Trinity";
        return;
    }
}

static void applyPrettyNameIfWM(FFWMDEResult* result, const char* processName, ProtocolHint* protocolHint)
{
    if(strcasecmp(processName, "kwin_wayland") == 0)
    {
        ffStrbufSetS(&result->wmPrettyName, "KWin");
        *protocolHint = FF_PROTOCOL_HINT_WAYLAND;
    }
    else if(strcasecmp(processName, "kwin_x11") == 0)
    {
        ffStrbufSetS(&result->wmPrettyName, "KWin");
        *protocolHint = FF_PROTOCOL_HINT_X11;
    }
    else if(strcasecmp(processName, "sway") == 0)
    {
        ffStrbufSetS(&result->wmPrettyName, "Sway");
        *protocolHint = FF_PROTOCOL_HINT_WAYLAND;
    }
    else if(strcasecmp(processName, "weston") == 0)
    {
        ffStrbufSetS(&result->wmPrettyName, "Weston");
        *protocolHint = FF_PROTOCOL_HINT_WAYLAND;
    }
    else if(strcasecmp(processName, "wayfire") == 0)
    {
        ffStrbufSetS(&result->wmPrettyName, "Wayfire");
        *protocolHint = FF_PROTOCOL_HINT_WAYLAND;
    }
    else if(strcasecmp(processName, "openbox") == 0)
    {
        ffStrbufSetS(&result->wmPrettyName, "Openbox");
        *protocolHint = FF_PROTOCOL_HINT_X11;
    }
    else if(strcasecmp(processName, "xfwm4") == 0)
    {
        ffStrbufSetS(&result->wmPrettyName, "Xfwm4");
        *protocolHint = FF_PROTOCOL_HINT_X11;
    }
    else if(strcasecmp(processName, "Marco") == 0)
    {
        ffStrbufSetS(&result->wmPrettyName, "Marco");
        *protocolHint = FF_PROTOCOL_HINT_X11;
    }
    else if(strcasecmp(processName, "gnome-session-binary") == 0 || strcasecmp(processName, "Mutter") == 0)
        ffStrbufSetS(&result->wmPrettyName, "Mutter");
    else if(strcasecmp(processName, "cinnamon-session") == 0 || strcasecmp(processName, "Muffin") == 0)
        ffStrbufSetS(&result->wmPrettyName, "Muffin");

    if(result->wmPrettyName.length > 0 && result->wmProcessName.length == 0)
        ffStrbufSetS(&result->wmProcessName, processName);
}

static void applyBetterWM(FFWMDEResult* result, const char* processName, ProtocolHint* protocolHint)
{
    if(processName == NULL || *processName == '\0')
        return;

    ffStrbufSetS(&result->wmProcessName, processName);
    applyPrettyNameIfWM(result, processName, protocolHint);
    if(result->wmPrettyName.length == 0)
        ffStrbufAppend(&result->wmPrettyName, &result->wmProcessName);
}

static void getKDE(FFWMDEResult* result, ProtocolHint* protocolHint)
{
    ffStrbufSetS(&result->deProcessName, "plasmashell");
    ffStrbufSetS(&result->dePrettyName, "KDE Plasma");
    ffParsePropFile("/usr/share/xsessions/plasma.desktop", "X-KDE-PluginInfo-Version =", &result->deVersion);
    applyBetterWM(result, getenv("KDEWM"), protocolHint);
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
    if(result->deVersion.length > 0)
    {
        ffStrbufAppendC(&result->deVersion, '.');
        ffParsePropFile("/usr/share/mate-about/mate-version.xml", "<minor>", &result->deVersion);
        ffStrbufAppendC(&result->deVersion, '.');
        ffParsePropFile("/usr/share/mate-about/mate-version.xml", "<micro>", &result->deVersion);
    }

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
        //This is somewhat slow
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

static void getLXQt(FFinstance* instance, FFWMDEResult* result, ProtocolHint* protocolHint)
{
    ffStrbufSetS(&result->deProcessName, "lxqt-session");
    ffStrbufSetS(&result->dePrettyName, "LXQt");
    ffParsePropFile("/usr/lib/pkgconfig/lxqt.pc", "Version:", &result->deVersion);

    if(result->deVersion.length == 0)
        ffParsePropFile("/usr/share/cmake/lxqt/lxqt-config.cmake", "set ( LXQT_VERSION", &result->deVersion);
    if(result->deVersion.length == 0)
        ffParsePropFile("/usr/share/cmake/lxqt/lxqt-config-version.cmake", "set ( PACKAGE_VERSION", &result->deVersion);

    if(result->deVersion.length == 0 && instance->config.allowSlowOperations)
    {
        //This is really, really, really slow. Thank you, LXQt developers
        ffProcessAppendStdOut(&result->deVersion, (char* const[]){
            "lxqt-session",
            "-v",
            NULL
        });

        result->deVersion.length = 0; //don't set '\0' byte
        ffGetPropValueFromLines(result->deVersion.chars , "liblxqt", &result->deVersion);
    }

    FFstrbuf wmProcessNameBuffer;
    ffStrbufInit(&wmProcessNameBuffer);

    ffParsePropFileConfig(instance, "lxqt/session.conf", "window_manager =", &wmProcessNameBuffer);
    applyBetterWM(result, wmProcessNameBuffer.chars, protocolHint);

    ffStrbufDestroy(&wmProcessNameBuffer);
}

static void applyPrettyNameIfDE(FFinstance* instance, FFWMDEResult* result, const char* name, ProtocolHint* protocolHint)
{
    if(name == NULL || *name == '\0')
        return;

    if(strcasecmp(name, "KDE") == 0 || strcasecmp(name, "plasma") == 0 || strcasecmp(name, "plasmashell") == 0)
        getKDE(result, protocolHint);
    else if(strcasecmp(name, "Gnome") == 0 || strcasecmp(name, "ubuntu:GNOME") == 0 || strcasecmp(name, "ubuntu") == 0 || strcasecmp(name, "gnome-shell") == 0)
        getGnome(result);
    else if(strcasecmp(name, "X-Cinnamon") == 0 || strcasecmp(name, "Cinnamon") == 0)
        getCinnamon(result);
    else if(strcasecmp(name, "XFCE") == 0 || strcasecmp(name, "X-XFCE") == 0 || strcasecmp(name, "XFCE4") == 0 || strcasecmp(name, "X-XFCE4") == 0 || strcasecmp(name, "xfce4-session") == 0)
        getXFCE4(instance, result);
    else if(strcasecmp(name, "MATE") == 0 || strcasecmp(name, "X-MATE") == 0 || strcasecmp(name, "mate-session") == 0)
        getMate(instance, result);
    else if(strcasecmp(name, "LXQt") == 0 || strcasecmp(name, "X-LXQT") == 0 || strcasecmp(name, "lxqt-session") == 0)
        getLXQt(instance, result, protocolHint);
}

static void getSessionTypeFromHint(FFWMDEResult* result, ProtocolHint protocolHint)
{
    if(
        result->wmProtocolName.length > 0 ||
        protocolHint == FF_PROTOCOL_HINT_UNKNOWN
    ) return;

    if(protocolHint == FF_PROTOCOL_HINT_WAYLAND)
        ffStrbufSetS(&result->wmProtocolName, "Wayland");
    else if(protocolHint == FF_PROTOCOL_HINT_X11)
        ffStrbufSetS(&result->wmProtocolName, "X11");
}

static void getSessionTypeFromEnv(FFWMDEResult* result)
{
    const char* env = getenv("XDG_SESSION_TYPE");
    if(env != NULL && *env != '\0')
    {
        if(strcasecmp(env, "wayland") == 0)
            ffStrbufSetS(&result->wmProtocolName, "Wayland");
        else if(strcasecmp(env, "x11") == 0)
            ffStrbufSetS(&result->wmProtocolName, "X11");
        else if(strcasecmp(env, "tty") == 0)
            ffStrbufSetS(&result->wmProtocolName, "TTY");
        else if(strcasecmp(env, "mir") == 0)
            ffStrbufSetS(&result->wmProtocolName, "Mir");
        else
            ffStrbufSetS(&result->wmProtocolName, env);

        return;
    }

    env = getenv("WAYLAND_DISPLAY");
    if(env != NULL && *env != '\0')
    {
        ffStrbufSetS(&result->wmProtocolName, "Wayland");
        return;
    }

    env = getenv("TERM");
    if(env != NULL && *env != '\0')
    {
        if(strcasecmp(env, "linux") == 0)
        {
            ffStrbufSetS(&result->wmProtocolName, "TTY");
            return;
        }
    }
}

static void getFromProcDir(FFinstance* instance, FFWMDEResult* result, ProtocolHint* protocolHint)
{
    DIR* proc = opendir("/proc");
    if(proc == NULL)
        return;

    FFstrbuf procPath;
    ffStrbufInitA(&procPath, 64);
    ffStrbufAppendS(&procPath, "/proc/");

    uint32_t procPathLength = procPath.length;

    FFstrbuf processName;
    ffStrbufInitA(&processName, 256); //Some processes have large command lines (looking at you chrome)

    struct dirent* dirent;
    while((dirent = readdir(proc)) != NULL)
    {
        if(dirent->d_type != DT_DIR || dirent->d_name[0] == '.')
            continue;

        ffStrbufAppendS(&procPath, dirent->d_name);
        ffStrbufAppendS(&procPath, "/cmdline");
        ffGetFileContent(procPath.chars, &processName);
        ffStrbufSubstrBeforeFirstC(&processName, '\0'); //Trim the arguments
        ffStrbufSubstrAfterLastC(&processName, '/');
        ffStrbufSubstrBefore(&procPath, procPathLength);

        if(result->dePrettyName.length == 0)
            applyPrettyNameIfDE(instance, result, processName.chars, protocolHint);

        if(result->wmPrettyName.length == 0)
            applyPrettyNameIfWM(result, processName.chars, protocolHint);

        if(result->deProcessName.length > 0 && result->wmPrettyName.length > 0)
            break;
    }

    closedir(proc);

    ffStrbufDestroy(&processName);
    ffStrbufDestroy(&procPath);
}

void getWMDE(FFinstance* instance, FFWMDEResult* result)
{
    ProtocolHint protocolHint;

    //If sessionDesktop is a known DE, set it. Some DEs even have config files which tell us the WM, so we can return very fast in this case
    applyPrettyNameIfDE(instance, result, result->sessionDesktop, &protocolHint);
    if(result->dePrettyName.length > 0 && result->wmPrettyName.length > 0)
    {
        getSessionTypeFromHint(result, protocolHint);
        return;
    }

    //If sessionDesktop is a known WM, set it.
    applyPrettyNameIfWM(result, result->sessionDesktop, &protocolHint);

    //Search for missing DE / WM in processes
    getFromProcDir(instance, result, &protocolHint);

    //Fallback for unknown WMs and DEs. This will falsely detect DEs as WMs if they (and their used WM) are unknown
    if(result->wmPrettyName.length == 0 && result->dePrettyName.length == 0)
    {
        ffStrbufSetS(&result->wmProcessName, result->sessionDesktop);
        ffStrbufSetS(&result->wmPrettyName, result->sessionDesktop);
    }

    getSessionTypeFromHint(result, protocolHint);
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

    getSessionDesktop(&result);
    getSessionTypeFromEnv(&result);

    //Don't run anyting when on TTY. This prevents us to catch process from other users in at least that case.
    if(ffStrbufIgnCaseCompS(&result.wmProtocolName, "TTY") != 0)
        getWMDE(instance, &result);

    pthread_mutex_unlock(&mutex);
    return &result;
}
