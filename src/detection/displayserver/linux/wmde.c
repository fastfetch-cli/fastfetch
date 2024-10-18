#include "displayserver_linux.h"
#include "common/io/io.h"
#include "common/properties.h"
#include "util/stringUtils.h"
#include "util/mallocHelper.h"

#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifdef __FreeBSD__
    #include <sys/sysctl.h>
    #include <sys/types.h>
    #include <sys/user.h>
#elif defined(__OpenBSD__)
    #include <sys/param.h>
    #include <sys/sysctl.h>
    #include <kvm.h>
#elif defined(__sun)
    #include <procfs.h>
#endif

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
        return "GNOME";

    if(getenv("MATE_DESKTOP_SESSION_ID") != NULL)
        return "Mate";

    if(getenv("TDE_FULL_SESSION") != NULL)
        return "Trinity";

    if(getenv("HYPRLAND_CMD") != NULL)
        return "Hyprland";

    #ifdef __linux__
    if(
        getenv("WAYLAND_DISPLAY") != NULL &&
        ffPathExists("/mnt/wslg/", FF_PATHTYPE_DIRECTORY)
    ) return "WSLg";
    #endif

    return NULL;
}

static void applyPrettyNameIfWM(FFDisplayServerResult* result, const char* name)
{
    if(!ffStrSet(name))
        return;

    if(
        ffStrEqualsIgnCase(name, "kwin") ||
        ffStrStartsWithIgnCase(name, "kwin_") ||
        ffStrEndsWithIgnCase(name, "-kwin_wayland") ||
        ffStrEndsWithIgnCase(name, "-kwin_x11")
    ) ffStrbufSetS(&result->wmPrettyName, FF_WM_PRETTY_KWIN);
    else if(
        ffStrEqualsIgnCase(name, "gnome-shell") ||
        ffStrEqualsIgnCase(name, "gnome shell") ||
        ffStrEqualsIgnCase(name, "gnome-session-binary") ||
        ffStrEqualsIgnCase(name, "Mutter")
    ) ffStrbufSetS(&result->wmPrettyName, FF_WM_PRETTY_MUTTER);
    else if(
        ffStrEqualsIgnCase(name, "cinnamon") ||
        ffStrStartsWithIgnCase(name, "cinnamon-") ||
        ffStrEqualsIgnCase(name, "Muffin") ||
        ffStrEqualsIgnCase(name, "Mutter (Muffin)")
    ) ffStrbufSetS(&result->wmPrettyName, FF_WM_PRETTY_MUFFIN);
    else if(ffStrEqualsIgnCase(name, "sway"))
        ffStrbufSetS(&result->wmPrettyName, FF_WM_PRETTY_SWAY);
    else if(ffStrEqualsIgnCase(name, "weston"))
        ffStrbufSetS(&result->wmPrettyName, FF_WM_PRETTY_WESTON);
    else if(ffStrEqualsIgnCase(name, "wayfire"))
        ffStrbufSetS(&result->wmPrettyName, FF_WM_PRETTY_WAYFIRE);
    else if(ffStrEqualsIgnCase(name, "openbox"))
        ffStrbufSetS(&result->wmPrettyName, FF_WM_PRETTY_OPENBOX);
    else if(ffStrEqualsIgnCase(name, "xfwm4"))
        ffStrbufSetS(&result->wmPrettyName, FF_WM_PRETTY_XFWM4);
    else if(ffStrEqualsIgnCase(name, "Marco") ||
        ffStrEqualsIgnCase(name, "Metacity (Marco)"))
        ffStrbufSetS(&result->wmPrettyName, FF_WM_PRETTY_MARCO);
    else if(ffStrEqualsIgnCase(name, "xmonad"))
        ffStrbufSetS(&result->wmPrettyName, FF_WM_PRETTY_XMONAD);
    else if(ffStrEqualsIgnCase(name, "WSLg"))
        ffStrbufSetS(&result->wmPrettyName, FF_WM_PRETTY_WSLG);
    else if(ffStrEqualsIgnCase(name, "dwm"))
        ffStrbufSetS(&result->wmPrettyName, FF_WM_PRETTY_DWM);
    else if(ffStrEqualsIgnCase(name, "bspwm"))
        ffStrbufSetS(&result->wmPrettyName, FF_WM_PRETTY_BSPWM);
    else if(ffStrEqualsIgnCase(name, "tinywm"))
        ffStrbufSetS(&result->wmPrettyName, FF_WM_PRETTY_TINYWM);
    else if(ffStrEqualsIgnCase(name, "qtile"))
        ffStrbufSetS(&result->wmPrettyName, FF_WM_PRETTY_QTILE);
    else if(ffStrEqualsIgnCase(name, "herbstluftwm"))
        ffStrbufSetS(&result->wmPrettyName, FF_WM_PRETTY_HERBSTLUFTWM);
    else if(ffStrEqualsIgnCase(name, "icewm"))
        ffStrbufSetS(&result->wmPrettyName, FF_WM_PRETTY_ICEWM);
    else if(ffStrEqualsIgnCase(name, "dtwm"))
        ffStrbufSetS(&result->wmPrettyName, FF_WM_PRETTY_DTWM);
    else if(ffStrEqualsIgnCase(name, "hyprland"))
        ffStrbufSetS(&result->wmPrettyName, FF_WM_PRETTY_HYPRLAND);
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
        ffStrEqualsIgnCase(name, "KDE") ||
        ffStrEqualsIgnCase(name, "plasma") ||
        ffStrEqualsIgnCase(name, "plasmashell") ||
        ffStrEqualsIgnCase(name, "plasmawayland")
    ) {
        ffStrbufSetStatic(&result->deProcessName, "plasmashell");
        ffStrbufSetStatic(&result->dePrettyName, FF_DE_PRETTY_PLASMA);
        applyBetterWM(result, getenv("KDEWM"));
    }

    else if(
        ffStrEqualsIgnCase(name, "GNOME") ||
        ffStrEqualsIgnCase(name, "ubuntu:GNOME") ||
        ffStrEqualsIgnCase(name, "ubuntu") ||
        ffStrEqualsIgnCase(name, "gnome-shell")
    ) {
        ffStrbufSetStatic(&result->deProcessName, "gnome-shell");
        const char* sessionMode = getenv("GNOME_SHELL_SESSION_MODE");
        if (sessionMode && ffStrEquals(sessionMode, "classic"))
            ffStrbufSetStatic(&result->dePrettyName, FF_DE_PRETTY_GNOME_CLASSIC);
        else
            ffStrbufSetStatic(&result->dePrettyName, FF_DE_PRETTY_GNOME);
    }

    else if(
        ffStrEqualsIgnCase(name, "X-Cinnamon") ||
        ffStrEqualsIgnCase(name, "Cinnamon")
    ) {
        ffStrbufSetS(&result->deProcessName, "cinnamon");
        ffStrbufSetS(&result->dePrettyName, FF_DE_PRETTY_CINNAMON);
    }

    else if(
        ffStrEqualsIgnCase(name, "XFCE") ||
        ffStrEqualsIgnCase(name, "X-XFCE") ||
        ffStrEqualsIgnCase(name, "XFCE4") ||
        ffStrEqualsIgnCase(name, "X-XFCE4") ||
        ffStrEqualsIgnCase(name, "xfce4-session")
    ) {
        ffStrbufSetS(&result->deProcessName, "xfce4-session");
        ffStrbufSetS(&result->dePrettyName, FF_DE_PRETTY_XFCE4);
    }

    else if(
        ffStrEqualsIgnCase(name, "MATE") ||
        ffStrEqualsIgnCase(name, "X-MATE") ||
        ffStrEqualsIgnCase(name, "mate-session")
    ) {
        ffStrbufSetS(&result->deProcessName, "mate-session");
        ffStrbufSetS(&result->dePrettyName, FF_DE_PRETTY_MATE);
    }

    else if(
        ffStrEqualsIgnCase(name, "LXQt") ||
        ffStrEqualsIgnCase(name, "X-LXQt") ||
        ffStrEqualsIgnCase(name, "lxqt-session")
    ) {
        ffStrbufSetS(&result->deProcessName, "lxqt-session");
        ffStrbufSetS(&result->dePrettyName, FF_DE_PRETTY_LXQT);
        FF_STRBUF_AUTO_DESTROY wmProcessNameBuffer = ffStrbufCreate();
        ffParsePropFileConfig("lxqt/session.conf", "window_manager =", &wmProcessNameBuffer);
        applyBetterWM(result, wmProcessNameBuffer.chars);
    }

    else if(
        ffStrEqualsIgnCase(name, "Budgie") ||
        ffStrEqualsIgnCase(name, "X-Budgie") ||
        ffStrEqualsIgnCase(name, "budgie-desktop") ||
        ffStrEqualsIgnCase(name, "Budgie:GNOME")
    ) {
        ffStrbufSetS(&result->deProcessName, "budgie-desktop");
        ffStrbufSetS(&result->dePrettyName, FF_DE_PRETTY_BUDGIE);
    }

    else if(
        ffStrEqualsIgnCase(name, "dtsession")
    ) {
        ffStrbufSetS(&result->deProcessName, "dtsession");
        ffStrbufSetS(&result->dePrettyName, FF_DE_PRETTY_CDE);
    }

    else if(
        ffStrEqualsIgnCase(name, "ukui-session")
    ) {
        ffStrbufSetS(&result->deProcessName, "ukui-session");
        ffStrbufSetS(&result->dePrettyName, FF_DE_PRETTY_UKUI);
    }

    else if(
        ffStrStartsWithIgnCase(name, "Unity:Unity")
    ) {
        ffStrbufSetS(&result->deProcessName, "unity-session");
        ffStrbufSetS(&result->dePrettyName, FF_DE_PRETTY_UNITY);
    }
}


static const char* getFromProcesses(FFDisplayServerResult* result)
{
    uint32_t userId = getuid();

#if __FreeBSD__
    int request[] = {CTL_KERN, KERN_PROC, KERN_PROC_UID, (int) userId};
    size_t length = 0;

    if(sysctl(request, ARRAY_SIZE(request), NULL, &length, NULL, 0) != 0)
        return "sysctl({CTL_KERN, KERN_PROC, KERN_PROC_UID}, NULL) failed";

    FF_AUTO_FREE struct kinfo_proc* procs = (struct kinfo_proc*) malloc(length);
    if(sysctl(request, ARRAY_SIZE(request), procs, &length, NULL, 0) != 0)
        return "sysctl({CTL_KERN, KERN_PROC, KERN_PROC_UID}, procs) failed";

    length /= sizeof(*procs);

    for (struct kinfo_proc* proc = procs; proc < procs + length; ++proc)
    {
        if(result->dePrettyName.length == 0)
            applyPrettyNameIfDE(result, proc->ki_comm);

        if(result->wmPrettyName.length == 0)
            applyNameIfWM(result, proc->ki_comm);

        if(result->dePrettyName.length > 0 && result->wmPrettyName.length > 0)
            break;
    }
#elif __OpenBSD__
    kvm_t* kd = kvm_open(NULL, NULL, NULL, KVM_NO_FILES, NULL);
    int count = 0;
    const struct kinfo_proc* proc = kvm_getprocs(kd, KERN_PROC_UID, userId, sizeof(*proc), &count);
    if (proc)
    {
        for (int i = 0; i < count; ++i)
        {
            if(result->dePrettyName.length == 0)
                applyPrettyNameIfDE(result, proc->p_comm);

            if(result->wmPrettyName.length == 0)
                applyNameIfWM(result, proc->p_comm);

            if(result->dePrettyName.length > 0 && result->wmPrettyName.length > 0)
                break;
        }
    }
    kvm_close(kd);
#elif __sun
    FF_AUTO_CLOSE_DIR DIR* procdir = opendir("/proc");
    if(procdir == NULL)
        return "opendir(\"/proc\") failed";

    FF_STRBUF_AUTO_DESTROY procPath = ffStrbufCreateA(64);
    ffStrbufAppendS(&procPath, "/proc/");

    uint32_t procPathLength = procPath.length;

    struct dirent* dirent;
    while((dirent = readdir(procdir)) != NULL)
    {
        if (!ffCharIsDigit(dirent->d_name[0]))
            continue;

        ffStrbufAppendS(&procPath, dirent->d_name);
        ffStrbufAppendS(&procPath, "/psinfo");
        psinfo_t proc;
        if (ffReadFileData(procPath.chars, sizeof(proc), &proc) == sizeof(proc))
        {
            ffStrbufSubstrBefore(&procPath, procPathLength);

            if (proc.pr_uid != userId)
                continue;

            if(result->dePrettyName.length == 0)
                applyPrettyNameIfDE(result, proc.pr_fname);

            if(result->wmPrettyName.length == 0)
                applyNameIfWM(result, proc.pr_fname);

            if(result->dePrettyName.length > 0 && result->wmPrettyName.length > 0)
                break;
        }
    }
#elif __linux__
    FF_AUTO_CLOSE_DIR DIR* procdir = opendir("/proc");
    if(procdir == NULL)
        return "opendir(\"/proc\") failed";

    FF_STRBUF_AUTO_DESTROY procPath = ffStrbufCreateA(64);
    ffStrbufAppendS(&procPath, "/proc/");

    uint32_t procPathLength = procPath.length;

    FF_STRBUF_AUTO_DESTROY loginuid = ffStrbufCreate();
    FF_STRBUF_AUTO_DESTROY processName = ffStrbufCreateA(256); //Some processes have large command lines (looking at you chrome)

    struct dirent* dirent;
    while((dirent = readdir(procdir)) != NULL)
    {
        //Match only folders starting with a number (the pid folders)
        if(dirent->d_type != DT_DIR || !ffCharIsDigit(dirent->d_name[0]))
            continue;

        ffStrbufAppendS(&procPath, dirent->d_name);
        uint32_t procFolderPathLength = procPath.length;

        //Don't check for processes not owend by the current user.
        ffStrbufAppendS(&procPath, "/loginuid");
        ffReadFileBuffer(procPath.chars, &loginuid);
        if(ffStrbufToUInt(&loginuid, (uint64_t) -1) != userId)
        {
            ffStrbufSubstrBefore(&procPath, procPathLength);
            continue;
        }

        ffStrbufSubstrBefore(&procPath, procFolderPathLength);

        //We check the cmdline for the process name, because it is not trimmed.
        ffStrbufAppendS(&procPath, "/cmdline");
        ffReadFileBuffer(procPath.chars, &processName);
        ffStrbufTrimRightSpace(&processName);
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
#endif

    return NULL;
}

void ffdsDetectWMDE(FFDisplayServerResult* result)
{
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
    //This way we never call getFromProcDir(), which has slow initialization time
    if(result->dePrettyName.length > 0 && result->wmPrettyName.length > 0)
        return;

    //Get missing WM / DE from processes.
    getFromProcesses(result);

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
