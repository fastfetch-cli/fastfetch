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

    char cmdlineFilePath[64];
    snprintf(cmdlineFilePath, sizeof(cmdlineFilePath), "/proc/%d/cmdline", (int)pid);

    if(ffAppendFileBuffer(cmdlineFilePath, exe))
    {
        ffStrbufTrimRightSpace(exe);
        ffStrbufRecalculateLength(exe); //Trim the arguments
        ffStrbufTrimLeft(exe, '-'); //Login shells start with a dash
    }

    #elif defined(__APPLE__)

    size_t len = 0;
    int mibs[] = { CTL_KERN, KERN_PROCARGS2, pid };
    if (sysctl(mibs, sizeof(mibs) / sizeof(*mibs), NULL, &len, NULL, 0) == 0)
    {
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

static const char* getProcessNameAndPpid(pid_t pid, char* name, pid_t* ppid)
{
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
    if(
        sscanf(buf, "%*s (%255[^)]) %*c %d", name, ppid) != 2 || //stat (comm) state ppid
        !ffStrSet(name) ||
        *ppid == 0
    )
        return "sscanf(stat) failed";

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
    strncpy(name, proc.kp_proc.p_comm, MAXCOMLEN); //trancated to 16 chars

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
    strncpy(name, proc.ki_comm, COMMLEN);

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

    while (getProcessNameAndPpid(pid, name, &ppid) == NULL)
    {
        //Common programs that are between terminal and own process, but are not the shell
        if(
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
            ffStrContainsIgnCase(name, "debug")      ||
            ffStrContainsIgnCase(name, "not-found")  ||
            ffStrEndsWith(name, ".sh")
        )
        {
            pid = ppid;
            continue;
        }

        result->pid = (uint32_t) pid;
        result->ppid = (uint32_t) ppid;
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

    while (getProcessNameAndPpid(pid, name, &ppid) == NULL)
    {
        //Known shells
        if (
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
            ffStrEndsWith(name, ".sh")
        )
        {
            pid = ppid;
            continue;
        }

        #ifdef __APPLE__
        // https://github.com/fastfetch-cli/fastfetch/discussions/501
        if (ffStrEndsWith(name, " (figterm)") || ffStrEndsWith(name, " (cwterm)"))
        {
            if (__builtin_expect(getProcessNameAndPpid(ppid, name, &ppid) != NULL, false))
                return 0;
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
        #endif

        !ffStrbufEqualS(&result->processName, "0")
    ) return;

    const char* term = NULL;

    //SSH
    if(getenv("SSH_CONNECTION") != NULL)
        term = getenv("SSH_TTY");

    #ifdef __linux__
    //Windows Terminal
    if(!ffStrSet(term) && (
        getenv("WT_SESSION") != NULL ||
        getenv("WT_PROFILE_ID") != NULL
    )) term = "Windows Terminal";

    //ConEmu
    if(!ffStrSet(term) && (
        getenv("ConEmuPID") != NULL
    )) term = "ConEmu";
    #endif

    //Alacritty
    if(!ffStrSet(term) && (
        getenv("ALACRITTY_SOCKET") != NULL ||
        getenv("ALACRITTY_LOG") != NULL ||
        getenv("ALACRITTY_WINDOW_ID") != NULL
    )) term = "Alacritty";

    #ifdef __ANDROID__
    //Termux
    if(!ffStrSet(term) && (
        getenv("TERMUX_VERSION") != NULL ||
        getenv("TERMUX_MAIN_PACKAGE_FORMAT") != NULL
    )) term = "com.termux";
    #endif

    #ifdef __linux__
    //Konsole
    if(!ffStrSet(term) && (
        getenv("KONSOLE_VERSION") != NULL
    )) term = "konsole";
    #endif

    //MacOS, mintty
    if(!ffStrSet(term))
        term = getenv("TERM_PROGRAM");

    if(!ffStrSet(term))
        term = getenv("LC_TERMINAL");

    //Normal Terminal
    if(!ffStrSet(term))
        term = getenv("TERM");

    //TTY
    if(!ffStrSet(term) || ffStrEquals(term, "linux"))
        term = ttyname(STDIN_FILENO);

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
    if(result->exeName[0] == '.' && ffStrEndsWith(result->exeName, "-wrapped"))
    {
        // For NixOS. Ref: #510 and https://github.com/NixOS/nixpkgs/pull/249428
        // We use processName when detecting version and font, overriding it for simplification
        ffStrbufSetNS(
            &result->processName,
            (uint32_t) (strlen(result->exeName) - strlen(".-wrapped")),
            result->exeName + 1);
    }

    if(ffStrbufEqualS(&result->processName, "wezterm-gui"))
        ffStrbufInitStatic(&result->prettyName, "WezTerm");
    else if(ffStrbufStartsWithS(&result->processName, "tmux:"))
        ffStrbufInitStatic(&result->prettyName, "tmux");

    #if defined(__ANDROID__)

    else if(ffStrbufEqualS(&result->processName, "com.termux"))
        ffStrbufInitStatic(&result->prettyName, "Termux");

    #elif defined(__linux__) || defined(__FreeBSD__)

    else if(ffStrbufStartsWithS(&result->processName, "gnome-terminal-"))
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
    result.pid = 0;
    result.ppid = 0;

    pid_t ppid = (pid_t) ffDetectShell()->ppid;

    if (ppid)
        ppid = getTerminalInfo(&result, ppid);
    getTerminalFromEnv(&result);
    setTerminalInfoDetails(&result);

    return &result;
}
