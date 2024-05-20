#include "terminalshell.h"
#include "common/io/io.h"
#include "common/parsing.h"
#include "common/processing.h"
#include "common/thread.h"
#include "util/stringUtils.h"
#include "util/mallocHelper.h"

#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#if defined(__FreeBSD__) || defined(__APPLE__)
    #include <sys/types.h>
    #include <sys/user.h>
    #include <sys/sysctl.h>
#endif

static void setExeName(FFstrbuf* exe, const char** exeName)
{
    assert(exe->length > 0);
    uint32_t lastSlashIndex = ffStrbufLastIndexC(exe, '/');
    if(lastSlashIndex < exe->length)
        *exeName = exe->chars + lastSlashIndex + 1;
}

static void getProcessInformation(pid_t pid, FFstrbuf* processName, FFstrbuf* exe, const char** exeName, FFstrbuf* exePath)
{
    assert(processName->length > 0);
    ffStrbufClear(exe);

    #ifdef __linux__

    char filePath[64];
    snprintf(filePath, sizeof(filePath), "/proc/%d/cmdline", (int)pid);

    if(ffAppendFileBuffer(filePath, exe))
    {
        ffStrbufTrimRightSpace(exe);
        ffStrbufRecalculateLength(exe); //Trim the arguments
        ffStrbufTrimLeft(exe, '-'); //Login shells start with a dash
    }

    snprintf(filePath, sizeof(filePath), "/proc/%d/exe", (int)pid);
    ffStrbufEnsureFixedLengthFree(exePath, PATH_MAX);
    ssize_t length = readlink(filePath, exePath->chars, exePath->allocated - 1);
    if (length > 0) // doesn't contain trailing NUL
    {
        exePath->chars[length] = '\0';
        exePath->length = (uint32_t) length;
    }

    #elif defined(__APPLE__)

    size_t len = 0;
    int mibs[] = { CTL_KERN, KERN_PROCARGS2, pid };
    if (sysctl(mibs, sizeof(mibs) / sizeof(*mibs), NULL, &len, NULL, 0) == 0)
    {
        #ifndef MAC_OS_X_VERSION_10_15
        //don't know why if don't let len longer, proArgs2 and len will change during the following sysctl() in old MacOS version.
        len++;
        #endif
        FF_AUTO_FREE char* const procArgs2 = malloc(len);
        if (sysctl(mibs, sizeof(mibs) / sizeof(*mibs), procArgs2, &len, NULL, 0) == 0)
        {
            // https://gist.github.com/nonowarn/770696#file-getargv-c-L46
            uint32_t argc = *(uint32_t*) procArgs2;
            const char* realExePath = procArgs2 + sizeof(argc);

            const char* arg0 = memchr(realExePath, '\0', len - (size_t) (realExePath - procArgs2));
            ffStrbufSetNS(exePath, (uint32_t) (arg0 - realExePath), realExePath);

            do arg0++; while (*arg0 == '\0');
            assert(arg0 < procArgs2 + len);
            if (*arg0 == '-') arg0++; // Login shells

            ffStrbufSetS(exe, arg0);
        }
    }

    #elif defined(__FreeBSD__)

    size_t size = ARG_MAX;
    FF_AUTO_FREE char* args = malloc(size);

    static_assert(ARG_MAX > PATH_MAX, "");

    if(sysctl(
        (int[]){CTL_KERN, KERN_PROC, KERN_PROC_PATHNAME, pid}, 4,
        args, &size,
        NULL, 0
    ) == 0)
        ffStrbufSetNS(exePath, (uint32_t) (size - 1), args);

    size = ARG_MAX;
    if(sysctl(
        (int[]){CTL_KERN, KERN_PROC, KERN_PROC_ARGS, pid}, 4,
        args, &size,
        NULL, 0
    ) == 0)
    {
        char* arg0 = args;
        size_t arg0Len = strlen(args);
        if (size > arg0Len + 1)
        {
            char* p = (char*) memrchr(args, '/', arg0Len);
            if (p)
            {
                p++;
                if (ffStrStartsWith(p, "python")) // /usr/local/bin/python3.9 /home/carter/.local/bin/xonsh
                {
                    arg0 += arg0Len + 1;
                }
            }
        }
        if (arg0[0] == '-') arg0++;
        ffStrbufSetS(exe, arg0);
    }

    #endif

    if(exe->length == 0)
        ffStrbufSet(exe, processName);

    setExeName(exe, exeName);
}

static const char* getProcessNameAndPpid(pid_t pid, char* name, pid_t* ppid, int32_t* tty)
{
    if (pid <= 0)
        return "Invalid pid";

    #ifdef __linux__

    char statFilePath[64];
    snprintf(statFilePath, sizeof(statFilePath), "/proc/%d/stat", (int)pid);
    char buf[PROC_FILE_BUFFSIZ];
    ssize_t nRead = ffReadFileData(statFilePath, sizeof(buf) - 1, buf);
    if(nRead < 0)
        return "ffReadFileData(statFilePath, sizeof(buf)-1, buf) failed";
    buf[nRead] = '\0';

    *ppid = 0;
    static_assert(sizeof(*ppid) == sizeof(int), "");

    int tty_;
    if(
        sscanf(buf, "%*s (%255[^)]) %*c %d %*d %*d %d", name, ppid, &tty_) < 2 || //stat (comm) state ppid pgrp session tty
        !ffStrSet(name) ||
        *ppid == 0
    )
        return "sscanf(stat) failed";

    if (tty)
        *tty = tty_ & 0xFF;

    #elif defined(__APPLE__)

    struct kinfo_proc proc;
    size_t size = sizeof(proc);
    if(sysctl(
        (int[]){CTL_KERN, KERN_PROC, KERN_PROC_PID, pid}, 4,
        &proc, &size,
        NULL, 0
    ))
        return "sysctl(KERN_PROC_PID) failed";

    *ppid = (pid_t)proc.kp_eproc.e_ppid;
    strcpy(name, proc.kp_proc.p_comm); //trancated to 16 chars
    if (tty)
    {
        *tty = ((proc.kp_eproc.e_tdev >> 24) & 0xFF) == 0x10
            ? proc.kp_eproc.e_tdev & 0xFFFFFF
            : -1;
    }

    #elif defined(__FreeBSD__)

    struct kinfo_proc proc;
    size_t size = sizeof(proc);
    if(sysctl(
        (int[]){CTL_KERN, KERN_PROC, KERN_PROC_PID, pid}, 4,
        &proc, &size,
        NULL, 0
    ))
        return "sysctl(KERN_PROC_PID) failed";

    *ppid = (pid_t)proc.ki_ppid;
    strcpy(name, proc.ki_comm);
    if (tty)
    {
        if (proc.ki_tdev != NODEV && proc.ki_flag & P_CONTROLT)
        {
            const char* ttyName = devname(proc.ki_tdev, S_IFCHR);
            if (ffStrStartsWith(ttyName, "pts/"))
                *tty = (int32_t) strtol(ttyName + strlen("pts/"), NULL, 10);
            else
                *tty = -1;
        }
        else
            *tty = -1;
    }

    #else

    return "Unsupported platform";

    #endif

    return NULL;
}

static pid_t getShellInfo(FFShellResult* result, pid_t pid)
{
    char name[256];
    name[0] = '\0';

    pid_t ppid = 0;
    int32_t tty = -1;

    const char* userShellName = NULL;
    {
        uint32_t index = ffStrbufLastIndexC(&instance.state.platform.userShell, '/');
        if (index == instance.state.platform.userShell.length)
            userShellName = instance.state.platform.userShell.chars;
        else
            userShellName = instance.state.platform.userShell.chars + index + 1;
    }

    while (getProcessNameAndPpid(pid, name, &ppid, &tty) == NULL)
    {
        if (!ffStrEquals(userShellName, name))
        {
            //Common programs that are between terminal and own process, but are not the shell
            if(
                // tty < 0                                  || //A shell should connect to a tty
                ffStrEquals(name, "sh")                  || //This prevents us from detecting things like pipes and redirects, i hope nobody uses plain `sh` as shell
                ffStrEquals(name, "sudo")                ||
                ffStrEquals(name, "su")                  ||
                ffStrEquals(name, "strace")              ||
                ffStrEquals(name, "sshd")                ||
                ffStrEquals(name, "gdb")                 ||
                ffStrEquals(name, "lldb")                ||
                ffStrEquals(name, "lldb-mi")             ||
                ffStrEquals(name, "login")               ||
                ffStrEquals(name, "ltrace")              ||
                ffStrEquals(name, "perf")                ||
                ffStrEquals(name, "guake-wrapped")       ||
                ffStrEquals(name, "time")                ||
                ffStrEquals(name, "hyfetch")             || //when hyfetch uses fastfetch as backend
                ffStrEquals(name, "clifm")               || //https://github.com/leo-arch/clifm/issues/289
                ffStrEquals(name, "valgrind")            ||
                ffStrContainsIgnCase(name, "debug")      ||
                ffStrContainsIgnCase(name, "not-found")  ||
                ffStrEndsWith(name, ".sh")
            )
            {
                pid = ppid;
                continue;
            }
        }

        result->pid = (uint32_t) pid;
        result->ppid = (uint32_t) ppid;
        result->tty = tty;
        ffStrbufSetS(&result->processName, name);
        getProcessInformation(pid, &result->processName, &result->exe, &result->exeName, &result->exePath);
        break;
    }
    return ppid;
}

static pid_t getTerminalInfo(FFTerminalResult* result, pid_t pid)
{
    char name[256];
    name[0] = '\0';

    pid_t ppid = 0;

    while (getProcessNameAndPpid(pid, name, &ppid, NULL) == NULL)
    {
        //Known shells
        if (
            ffStrEquals(name, "sh")         ||
            ffStrEquals(name, "ash")        ||
            ffStrEquals(name, "bash")       ||
            ffStrEquals(name, "zsh")        ||
            ffStrEquals(name, "ksh")        ||
            ffStrEquals(name, "mksh")       ||
            ffStrEquals(name, "oksh")       ||
            ffStrEquals(name, "csh")        ||
            ffStrEquals(name, "tcsh")       ||
            ffStrEquals(name, "fish")       ||
            ffStrEquals(name, "dash")       ||
            ffStrEquals(name, "pwsh")       ||
            ffStrEquals(name, "nu")         ||
            ffStrEquals(name, "git-shell")  ||
            ffStrEquals(name, "elvish")     ||
            ffStrEquals(name, "oil.ovm")    ||
            ffStrEquals(name, "xonsh")      || // works in Linux but not in macOS because kernel returns `Python` in this case
            ffStrEquals(name, "login")      ||
            ffStrEquals(name, "sshd")       ||
            ffStrEquals(name, "clifm")      || // https://github.com/leo-arch/clifm/issues/289
            ffStrEquals(name, "chezmoi")    || // #762
            #ifdef __linux__
            ffStrStartsWith(name, "flatpak-") || // #707
            #endif
            ffStrEndsWith(name, ".sh")
        )
        {
            pid = ppid;
            continue;
        }

        #ifdef __APPLE__
        // https://github.com/fastfetch-cli/fastfetch/discussions/501
        const char* pLeft = strstr(name, " (");
        if (pLeft)
        {
            pLeft += 2;
            const char* pRight = strstr(pLeft, "term)");
            if (pRight && pRight[5] == '\0')
            {
                for (; pLeft < pRight; ++pLeft)
                    if (*pLeft < 'a' || *pLeft > 'z')
                        break;
                if (pLeft == pRight && getProcessNameAndPpid(ppid, name, &ppid, NULL) != NULL)
                    return 0;
            }
        }
        #endif

        result->pid = (uint32_t) pid;
        result->ppid = (uint32_t) ppid;
        ffStrbufSetS(&result->processName, name);
        getProcessInformation(pid, &result->processName, &result->exe, &result->exeName, &result->exePath);
        break;
    }
    return ppid;
}

static bool getTerminalInfoByPidEnv(FFTerminalResult* result, const char* pidEnv)
{
    const char* envStr = getenv(pidEnv);
    if (envStr == NULL)
        return false;

    pid_t pid = (pid_t) strtol(envStr, NULL, 10);
    result->pid = (uint32_t) pid;
    char name[256];
    if (getProcessNameAndPpid(pid, name, (pid_t*) &result->ppid, NULL) == NULL)
    {
        ffStrbufSetS(&result->processName, name);
        getProcessInformation(pid, &result->processName, &result->exe, &result->exeName, &result->exePath);
        return true;
    }

    return false;
}

static void getTerminalFromEnv(FFTerminalResult* result)
{
    if(
        result->processName.length > 0 &&
        !ffStrbufStartsWithS(&result->processName, "login") &&
        !ffStrbufEqualS(&result->processName, "(login)") &&

        #ifdef __APPLE__
        !ffStrbufEqualS(&result->processName, "launchd") &&
        !ffStrbufEqualS(&result->processName, "stable") && //for WarpTerminal
        #else
        !ffStrbufEqualS(&result->processName, "systemd") &&
        !ffStrbufEqualS(&result->processName, "init") &&
        !ffStrbufEqualS(&result->processName, "(init)") &&
        !ffStrbufEqualS(&result->processName, "SessionLeader") && // #750
        #endif

        !ffStrbufEqualS(&result->processName, "0")
    ) return;

    const char* term = NULL;

    //SSH
    if(
        getenv("SSH_TTY") != NULL
    )
        term = getenv("SSH_TTY");
    else if(
        getenv("KITTY_PID") != NULL ||
        getenv("KITTY_INSTALLATION_DIR") != NULL
    )
    {
        if (getTerminalInfoByPidEnv(result, "KITTY_PID"))
            return;
        term = "kitty";
    }

    #ifdef __linux__ // WSL
    //Windows Terminal
    else if(
        getenv("WT_SESSION") != NULL ||
        getenv("WT_PROFILE_ID") != NULL
    ) term = "Windows Terminal";

    //ConEmu
    else if(
        getenv("ConEmuPID") != NULL
    ) term = "ConEmu";
    #endif

    //Alacritty
    else if(
        getenv("ALACRITTY_SOCKET") != NULL ||
        getenv("ALACRITTY_LOG") != NULL ||
        getenv("ALACRITTY_WINDOW_ID") != NULL
    ) term = "Alacritty";

    #ifdef __ANDROID__
    //Termux
    else if(
        getenv("TERMUX_VERSION") != NULL ||
        getenv("TERMUX_MAIN_PACKAGE_FORMAT") != NULL
    )
    {
        if (getTerminalInfoByPidEnv(result, "TERMUX_APP__PID"))
            return;
        term = "com.termux";
    }
    #endif

    #if defined(__linux__) || defined(__FreeBSD__)
    //Konsole
    else if(
        getenv("KONSOLE_VERSION") != NULL
    ) term = "konsole";

    else if(
        getenv("GNOME_TERMINAL_SCREEN") != NULL ||
        getenv("GNOME_TERMINAL_SERVICE") != NULL
    ) term = "gnome-terminal";
    #endif

    //MacOS, mintty
    else if(getenv("TERM_PROGRAM") != NULL)
        term = getenv("TERM_PROGRAM");

    else if(getenv("LC_TERMINAL") != NULL)
        term = getenv("LC_TERMINAL");

    //Normal Terminal
    else
    {
        term = getenv("TERM");
        //TTY
        if(!ffStrSet(term) || ffStrEquals(term, "linux"))
            term = ttyname(STDIN_FILENO);
    }

    if(ffStrSet(term))
    {
        ffStrbufSetS(&result->processName, term);
        ffStrbufSetS(&result->exe, term);
        setExeName(&result->exe, &result->exeName);
    }
}

static void getUserShellFromEnv(FFShellResult* result)
{
    //If shell detection via processes failed
    if(result->processName.length == 0 && instance.state.platform.userShell.length > 0)
    {
        ffStrbufSet(&result->exe, &instance.state.platform.userShell);
        setExeName(&result->exe, &result->exeName);
        ffStrbufAppendS(&result->processName, result->exeName);
    }
}

bool fftsGetShellVersion(FFstrbuf* exe, const char* exeName, FFstrbuf* version);

bool fftsGetTerminalVersion(FFstrbuf* processName, FFstrbuf* exe, FFstrbuf* version);

static void setShellInfoDetails(FFShellResult* result)
{
    ffStrbufClear(&result->version);
    fftsGetShellVersion(&result->exe, result->exeName, &result->version);

    if(ffStrbufEqualS(&result->processName, "pwsh"))
        ffStrbufInitStatic(&result->prettyName, "PowerShell");
    else if(ffStrbufEqualS(&result->processName, "nu"))
        ffStrbufInitStatic(&result->prettyName, "nushell");
    else if(ffStrbufEqualS(&result->processName, "oil.ovm"))
        ffStrbufInitStatic(&result->prettyName, "Oils");
    else
    {
        // https://github.com/fastfetch-cli/fastfetch/discussions/280#discussioncomment-3831734
        ffStrbufInitS(&result->prettyName, result->exeName);
    }
}

static void setTerminalInfoDetails(FFTerminalResult* result)
{
    if(ffStrbufStartsWithC(&result->processName, '.') && ffStrbufEndsWithS(&result->processName, "-wrapped"))
    {
        // For NixOS. Ref: #510 and https://github.com/NixOS/nixpkgs/pull/249428
        // We use processName when detecting version and font, overriding it for simplification
        ffStrbufSubstrBefore(&result->processName, result->processName.length - (uint32_t) strlen("-wrapped"));
        ffStrbufSubstrAfter(&result->processName, 0);
    }

    if(ffStrbufEqualS(&result->processName, "wezterm-gui"))
        ffStrbufInitStatic(&result->prettyName, "WezTerm");
    else if(ffStrbufStartsWithS(&result->processName, "tmux:"))
        ffStrbufInitStatic(&result->prettyName, "tmux");
    else if(ffStrbufStartsWithS(&result->processName, "screen-"))
        ffStrbufInitStatic(&result->prettyName, "screen");

    #if defined(__ANDROID__)

    else if(ffStrbufEqualS(&result->processName, "com.termux"))
        ffStrbufInitStatic(&result->prettyName, "Termux");

    #elif defined(__linux__) || defined(__FreeBSD__)

    else if(ffStrbufStartsWithS(&result->processName, "gnome-terminal"))
        ffStrbufInitStatic(&result->prettyName, "GNOME Terminal");
    else if(ffStrbufStartsWithS(&result->processName, "kgx"))
        ffStrbufInitStatic(&result->prettyName, "GNOME Console");
    else if(ffStrbufEqualS(&result->processName, "urxvt") ||
        ffStrbufEqualS(&result->processName, "urxvtd") ||
        ffStrbufEqualS(&result->processName, "rxvt")
    )
        ffStrbufInitStatic(&result->prettyName, "rxvt-unicode");

    #elif defined(__APPLE__)

    else if(ffStrbufEqualS(&result->processName, "iTerm.app") || ffStrbufStartsWithS(&result->processName, "iTermServer-"))
        ffStrbufInitStatic(&result->prettyName, "iTerm");
    else if(ffStrbufEndsWithS(&result->exePath, "Terminal.app/Contents/MacOS/Terminal"))
    {
        ffStrbufSetStatic(&result->processName, "Apple_Terminal"); // for terminal font detection
        ffStrbufInitStatic(&result->prettyName, "Apple Terminal");
    }
    else if(ffStrbufEqualS(&result->processName, "Apple_Terminal"))
        ffStrbufInitStatic(&result->prettyName, "Apple Terminal");
    else if(ffStrbufEqualS(&result->processName, "WarpTerminal"))
        ffStrbufInitStatic(&result->prettyName, "Warp");

    #endif

    else if(strncmp(result->exeName, result->processName.chars, result->processName.length) == 0) // if exeName starts with processName, print it. Otherwise print processName
        ffStrbufInitS(&result->prettyName, result->exeName);
    else
        ffStrbufInitCopy(&result->prettyName, &result->processName);

    fftsGetTerminalVersion(&result->processName, &result->exe, &result->version);
}

#if defined(MAXPATH)
#define FF_EXE_PATH_LEN MAXPATH
#elif defined(PATH_MAX)
#define FF_EXE_PATH_LEN PATH_MAX
#else
#define FF_EXE_PATH_LEN 260
#endif

const FFShellResult* ffDetectShell()
{
    static FFShellResult result;
    static bool init = false;
    if(init)
        return &result;
    init = true;

    ffStrbufInit(&result.processName);
    ffStrbufInitA(&result.exe, FF_EXE_PATH_LEN);
    result.exeName = result.exe.chars;
    ffStrbufInit(&result.exePath);
    ffStrbufInit(&result.version);
    result.pid = 0;
    result.ppid = 0;
    result.tty = -1;

    pid_t ppid = getppid();
    ppid = getShellInfo(&result, ppid);
    getUserShellFromEnv(&result);
    setShellInfoDetails(&result);

    return &result;
}

const FFTerminalResult* ffDetectTerminal()
{
    static FFTerminalResult result;
    static bool init = false;
    if(init)
        return &result;
    init = true;

    ffStrbufInit(&result.processName);
    ffStrbufInitA(&result.exe, FF_EXE_PATH_LEN);
    result.exeName = result.exe.chars;
    ffStrbufInit(&result.exePath);
    ffStrbufInit(&result.version);
    ffStrbufInitS(&result.tty, ttyname(STDOUT_FILENO));
    result.pid = 0;
    result.ppid = 0;

    pid_t ppid = (pid_t) ffDetectShell()->ppid;

    if (ppid)
        ppid = getTerminalInfo(&result, ppid);
    getTerminalFromEnv(&result);
    setTerminalInfoDetails(&result);

    return &result;
}
