#include "displayserver_linux.h"
#include "common/io/io.h"
#include "common/properties.h"
#include "common/processing.h"
#include "util/stringUtils.h"

#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>

static const char* parseEnv(void)
{
    const char* env;

    env = getenv("XDG_CURRENT_DESKTOP");
    if(ffStrSet(env))
        return env;

    env = getenv("XDG_SESSION_DESKTOP");
    if(ffStrSet(env))
        return env;

    env = getenv("CURRENT_DESKTOP");
    if(ffStrSet(env))
        return env;

    env = getenv("SESSION_DESKTOP");
    if(ffStrSet(env))
        return env;

    env = getenv("DESKTOP_SESSION");
    if(ffStrSet(env))
        return env;

    if(getenv("KDE_FULL_SESSION") != NULL || getenv("KDE_SESSION_UID") != NULL || getenv("KDE_SESSION_VERSION") != NULL)
        return "KDE";

    if(getenv("GNOME_DESKTOP_SESSION_ID") != NULL)
        return "Gnome";

    if(getenv("MATE_DESKTOP_SESSION_ID") != NULL)
        return "Mate";

    if(getenv("TDE_FULL_SESSION") != NULL)
        return "Trinity";

    if(
        getenv("WAYLAND_DISPLAY") != NULL &&
        ffPathExists("/mnt/wslg/", FF_PATHTYPE_DIRECTORY)
    ) return "WSLg";

    return NULL;
}

static void applyPrettyNameIfWM(FFDisplayServerResult* result, const char* name)
{
    if(!ffStrSet(name))
        return;

    if(
        strcasecmp(name, "kwin_wayland") == 0 ||
        strcasecmp(name, "kwin_wayland_wrapper") == 0 ||
        strcasecmp(name, "kwin_x11") == 0 ||
        strcasecmp(name, "kwin_x11_wrapper") == 0 ||
        strcasecmp(name, "kwin") == 0
    ) ffStrbufSetS(&result->wmPrettyName, FF_WM_PRETTY_KWIN);
    else if(
        strcasecmp(name, "gnome-shell") == 0 ||
        strcasecmp(name, "gnome shell") == 0 ||
        strcasecmp(name, "gnome-session-binary") == 0 ||
        strcasecmp(name, "Mutter") == 0
    ) ffStrbufSetS(&result->wmPrettyName, FF_WM_PRETTY_MUTTER);
    else if(
        strcasecmp(name, "cinnamon-session") == 0 ||
        strcasecmp(name, "Muffin") == 0 ||
        strcasecmp(name, "Mutter (Muffin)") == 0
    ) ffStrbufSetS(&result->wmPrettyName, FF_WM_PRETTY_MUFFIN);
    else if(strcasecmp(name, "sway") == 0)
        ffStrbufSetS(&result->wmPrettyName, FF_WM_PRETTY_SWAY);
    else if(strcasecmp(name, "weston") == 0)
        ffStrbufSetS(&result->wmPrettyName, FF_WM_PRETTY_WESTON);
    else if(strcasecmp(name, "wayfire") == 0)
        ffStrbufSetS(&result->wmPrettyName, FF_WM_PRETTY_WAYFIRE);
    else if(strcasecmp(name, "openbox") == 0)
        ffStrbufSetS(&result->wmPrettyName, FF_WM_PRETTY_OPENBOX);
    else if(strcasecmp(name, "xfwm4") == 0)
        ffStrbufSetS(&result->wmPrettyName, FF_WM_PRETTY_XFWM4);
    else if(strcasecmp(name, "Marco") == 0 ||
        strcasecmp(name, "Metacity (Marco)") == 0)
        ffStrbufSetS(&result->wmPrettyName, FF_WM_PRETTY_MARCO);
    else if(strcasecmp(name, "xmonad") == 0)
        ffStrbufSetS(&result->wmPrettyName, FF_WM_PRETTY_XMONAD);
    else if(strcasecmp(name, "WSLg") == 0)
        ffStrbufSetS(&result->wmPrettyName, FF_WM_PRETTY_WSLG);
    else if(strcasecmp(name, "dwm") == 0)
        ffStrbufSetS(&result->wmPrettyName, FF_WM_PRETTY_DWM);
    else if(strcasecmp(name, "bspwm") == 0)
        ffStrbufSetS(&result->wmPrettyName, FF_WM_PRETTY_BSPWM);
    else if(strcasecmp(name, "tinywm") == 0)
        ffStrbufSetS(&result->wmPrettyName, FF_WM_PRETTY_TINYWM);
    else if(strcasecmp(name, "qtile") == 0)
        ffStrbufSetS(&result->wmPrettyName, FF_WM_PRETTY_QTILE);
    else if(strcasecmp(name, "herbstluftwm") == 0)
        ffStrbufSetS(&result->wmPrettyName, FF_WM_PRETTY_HERBSTLUFTWM);
    else if(strcasecmp(name, "icewm") == 0)
        ffStrbufSetS(&result->wmPrettyName, FF_WM_PRETTY_ICEWM);
}

static void applyNameIfWM(FFDisplayServerResult* result, const char* processName)
{
    applyPrettyNameIfWM(result, processName);
    if(result->wmPrettyName.length > 0)
        ffStrbufSetS(&result->wmProcessName, processName);
}

static void applyBetterWM(FFDisplayServerResult* result, const char* processName)
{
    if(!ffStrSet(processName))
        return;

    ffStrbufSetS(&result->wmProcessName, processName);

    //If it is a known wm, this will set the pretty name
    applyPrettyNameIfWM(result, processName);

    //If it isn't a known wm, set the pretty name to the process name
    if(result->wmPrettyName.length == 0)
        ffStrbufAppend(&result->wmPrettyName, &result->wmProcessName);
}

static void applyPrettyNameIfDE(FFDisplayServerResult* result, const char* name)
{
    if(!ffStrSet(name))
        return;

    else if(
        strcasecmp(name, "KDE") == 0 ||
        strcasecmp(name, "plasma") == 0 ||
        strcasecmp(name, "plasmashell") == 0 ||
        strcasecmp(name, "plasmawayland") == 0
    ) {
        ffStrbufSetStatic(&result->deProcessName, "plasmashell");
        ffStrbufSetStatic(&result->dePrettyName, FF_DE_PRETTY_PLASMA);
        applyBetterWM(result, getenv("KDEWM"));
    }

    else if(
        strcasecmp(name, "Gnome") == 0 ||
        strcasecmp(name, "ubuntu:GNOME") == 0 ||
        strcasecmp(name, "ubuntu") == 0 ||
        strcasecmp(name, "gnome-shell") == 0
    ) {
        ffStrbufSetStatic(&result->deProcessName, "gnome-shell");
        const char* sessionMode = getenv("GNOME_SHELL_SESSION_MODE");
        if (sessionMode && ffStrEquals(sessionMode, "classic"))
            ffStrbufSetStatic(&result->dePrettyName, FF_DE_PRETTY_GNOME_CLASSIC);
        else
            ffStrbufSetStatic(&result->dePrettyName, FF_DE_PRETTY_GNOME);
    }

    else if(
        strcasecmp(name, "X-Cinnamon") == 0 ||
        strcasecmp(name, "Cinnamon") == 0
    ) {
        ffStrbufSetS(&result->deProcessName, "cinnamon");
        ffStrbufSetS(&result->dePrettyName, FF_DE_PRETTY_CINNAMON);
    }

    else if(
        strcasecmp(name, "XFCE") == 0 ||
        strcasecmp(name, "X-XFCE") == 0 ||
        strcasecmp(name, "XFCE4") == 0 ||
        strcasecmp(name, "X-XFCE4") == 0 ||
        strcasecmp(name, "xfce4-session") == 0
    ) {
        ffStrbufSetS(&result->deProcessName, "xfce4-session");
        ffStrbufSetS(&result->dePrettyName, FF_DE_PRETTY_XFCE4);
    }

    else if(
        strcasecmp(name, "MATE") == 0 ||
        strcasecmp(name, "X-MATE") == 0 ||
        strcasecmp(name, "mate-session") == 0
    ) {
        ffStrbufSetS(&result->deProcessName, "mate-session");
        ffStrbufSetS(&result->dePrettyName, FF_DE_PRETTY_MATE);
    }

    else if(
        strcasecmp(name, "LXQt") == 0 ||
        strcasecmp(name, "X-LXQT") == 0 ||
        strcasecmp(name, "lxqt-session") == 0
    ) {
        ffStrbufSetS(&result->deProcessName, "lxqt-session");
        ffStrbufSetS(&result->dePrettyName, FF_DE_PRETTY_LXQT);
        FF_STRBUF_AUTO_DESTROY wmProcessNameBuffer = ffStrbufCreate();
        ffParsePropFileConfig("lxqt/session.conf", "window_manager =", &wmProcessNameBuffer);
        applyBetterWM(result, wmProcessNameBuffer.chars);
    }

    else if(
        strcasecmp(name, "Budgie") == 0 ||
        strcasecmp(name, "X-Budgie") == 0 ||
        strcasecmp(name, "budgie-desktop") == 0 ||
        strcasecmp(name, "Budgie:GNOME") == 0
    ) {
        ffStrbufSetS(&result->deProcessName, "budgie-desktop");
        ffStrbufSetS(&result->dePrettyName, FF_DE_PRETTY_BUDGIE);
    }
}

static void getWMProtocolNameFromEnv(FFDisplayServerResult* result)
{
    //This is only called if all connection attempts to a display server failed
    //We don't need to check for wayland here, as the wayland code will always set the protocol name to wayland

    const char* env = getenv("XDG_SESSION_TYPE");
    if(ffStrSet(env))
    {
        if(strcasecmp(env, "x11") == 0)
            ffStrbufSetS(&result->wmProtocolName, FF_WM_PROTOCOL_X11);
        else if(strcasecmp(env, "tty") == 0)
            ffStrbufSetS(&result->wmProtocolName, FF_WM_PROTOCOL_TTY);
        else
            ffStrbufSetS(&result->wmProtocolName, env);

        return;
    }

    env = getenv("DISPLAY");
    if(ffStrSet(env))
    {
        ffStrbufSetS(&result->wmProtocolName, FF_WM_PROTOCOL_X11);
        return;
    }

    env = getenv("TERM");
    if(ffStrSet(env) && strcasecmp(env, "linux") == 0)
    {
        ffStrbufSetS(&result->wmProtocolName, FF_WM_PROTOCOL_TTY);
        return;
    }
}

static void getFromProcDir(FFDisplayServerResult* result)
{
    DIR* proc = opendir("/proc");
    if(proc == NULL)
        return;

    FF_STRBUF_AUTO_DESTROY procPath = ffStrbufCreateA(64);
    ffStrbufAppendS(&procPath, "/proc/");

    uint32_t procPathLength = procPath.length;

    FF_STRBUF_AUTO_DESTROY userID = ffStrbufCreateF("%i", getuid());
    FF_STRBUF_AUTO_DESTROY loginuid = ffStrbufCreate();
    FF_STRBUF_AUTO_DESTROY processName = ffStrbufCreateA(256); //Some processes have large command lines (looking at you chrome)

    struct dirent* dirent;
    while((dirent = readdir(proc)) != NULL)
    {
        //Match only folders starting with a number (the pid folders)
        if(dirent->d_type != DT_DIR || !isdigit(dirent->d_name[0]))
            continue;

        ffStrbufAppendS(&procPath, dirent->d_name);
        uint32_t procFolderPathLength = procPath.length;

        //Don't check for processes not owend by the current user.
        ffStrbufAppendS(&procPath, "/loginuid");
        ffReadFileBuffer(procPath.chars, &loginuid);
        if(ffStrbufComp(&userID, &loginuid) != 0)
        {
            ffStrbufSubstrBefore(&procPath, procPathLength);
            continue;
        }

        ffStrbufSubstrBefore(&procPath, procFolderPathLength);

        //We check the cmdline for the process name, because it is not trimmed.
        ffStrbufAppendS(&procPath, "/cmdline");
        ffReadFileBuffer(procPath.chars, &processName);
        ffStrbufSubstrBeforeFirstC(&processName, '\0'); //Trim the arguments
        ffStrbufSubstrAfterLastC(&processName, '/');

        ffStrbufSubstrBefore(&procPath, procPathLength);

        if(result->dePrettyName.length == 0)
            applyPrettyNameIfDE(result, processName.chars);

        if(result->wmPrettyName.length == 0)
            applyNameIfWM(result, processName.chars);

        if(result->dePrettyName.length > 0 && result->wmPrettyName.length > 0)
            break;
    }

    closedir(proc);
}

void ffdsDetectWMDE(FFDisplayServerResult* result)
{
    //If all connections failed, use the environment variables to detect protocol name
    if(result->wmProtocolName.length == 0)
        getWMProtocolNameFromEnv(result);

    //We don't want to detect anything in TTY
    //This can't happen if a connection succeeded, so we don't need to clear wmProcessName
    if(ffStrbufIgnCaseCompS(&result->wmProtocolName, FF_WM_PROTOCOL_TTY) == 0)
        return;

    const char* env = parseEnv();

    if(result->wmProcessName.length > 0)
    {
        //If we found the processName via display server, use it.
        //This will set the pretty name if it is a known WM, otherwise the prettyName to the processName
        applyPrettyNameIfWM(result, result->wmProcessName.chars);
        if(result->wmPrettyName.length == 0)
            ffStrbufSet(&result->wmPrettyName, &result->wmProcessName);
    }
    else
    {
        //if env is a known WM, use it
        applyNameIfWM(result, env);
    }

    //Connecting to a display server only gives WM results, not DE results.
    //If we find it in the environment, use that.
    applyPrettyNameIfDE(result, env);

    //If WM was found by connection to the sever, and DE in the environment, we can return
    //This way we never call getFromProcDir(), which has slow initalization time
    if(result->dePrettyName.length > 0 && result->wmPrettyName.length > 0)
        return;

    //Get missing WM / DE from processes.
    getFromProcDir(result);

    //Return if both wm and de are set, or if env doesn't contain anything
    if(
        (result->wmPrettyName.length > 0 && result->dePrettyName.length > 0) ||
        !ffStrSet(env)
    ) return;

    //If nothing is set, use env as WM
    else if(result->wmPrettyName.length == 0 && result->dePrettyName.length == 0)
    {
        ffStrbufSetS(&result->wmProcessName, env);
        ffStrbufSetS(&result->wmPrettyName, env);
    }

    //If only WM is not set, and DE doesn't equal env, use env as WM
    else if(
        result->wmPrettyName.length == 0 &&
        ffStrbufIgnCaseCompS(&result->deProcessName, env) != 0 &&
        ffStrbufIgnCaseCompS(&result->dePrettyName, env) != 0
    ) {
        ffStrbufSetS(&result->wmProcessName, env);
        ffStrbufSetS(&result->wmPrettyName, env);
    }

    //If only DE is not set, and WM doesn't equal env, use env as DE
    else if(
        result->dePrettyName.length == 0 &&
        ffStrbufIgnCaseCompS(&result->wmProcessName, env) != 0 &&
        ffStrbufIgnCaseCompS(&result->wmPrettyName, env) != 0
    ) {
        ffStrbufSetS(&result->deProcessName, env);
        ffStrbufSetS(&result->dePrettyName, env);
    }
}
