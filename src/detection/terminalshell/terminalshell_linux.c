#include "terminalshell.h"
#include "common/io/io.h"
#include "common/parsing.h"
#include "common/processing.h"
#include "common/thread.h"
#include "util/stringUtils.h"

#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#ifdef __APPLE__
    #include <libproc.h>
#elif defined(__FreeBSD__)
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

static void getProcessInformation(pid_t pid, FFstrbuf* processName, FFstrbuf* exe, const char** exeName)
{
    assert(processName->length > 0);
    ffStrbufClear(exe);

    #ifdef __linux__

    char cmdlineFilePath[64];
    snprintf(cmdlineFilePath, sizeof(cmdlineFilePath), "/proc/%d/cmdline", (int)pid);

    if(ffAppendFileBuffer(cmdlineFilePath, exe))
    {
        ffStrbufTrimRightSpace(exe);
        ffStrbufSubstrBeforeFirstC(exe, '\0'); //Trim the arguments
        ffStrbufTrimLeft(exe, '-'); //Happens in TTY
    }

    #elif defined(__APPLE__)

    int length = proc_pidpath((int)pid, exe->chars, exe->allocated);
    if(length > 0)
        exe->length = (uint32_t)length;

    #elif defined(__FreeBSD__)

    size_t size = exe->allocated;
    if(!sysctl(
        (int[]){CTL_KERN, KERN_PROC, KERN_PROC_PATHNAME, pid}, 4,
        exe->chars, &size,
        NULL, 0
    ))
        exe->length = (uint32_t)size - 1;

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
        return "ffReadFileData(statFilePath, sizeof(buf)-1, buf)";
    buf[nRead] = '\0';

    *ppid = 0;
    if(
        sscanf(buf, "%*s (%255[^)]) %*c %d", name, ppid) != 2 || //stat (comm) state ppid
        !ffStrSet(name) ||
        *ppid == 0
    )
        return "sscanf(stat) failed";

    #elif defined(__APPLE__)

    struct proc_bsdshortinfo proc;
    if(proc_pidinfo(pid, PROC_PIDT_SHORTBSDINFO, 0, &proc, PROC_PIDT_SHORTBSDINFO_SIZE) <= 0)
        return "proc_pidinfo(pid) failed";

    *ppid = (pid_t)proc.pbsi_ppid;
    strncpy(name, proc.pbsi_comm, 16); //trancated to 16 chars

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

static pid_t getShellInfo(FFTerminalShellResult* result, pid_t pid)
{
    char name[256];
    name[0] = '\0';

    pid_t ppid = 0;

    if(getProcessNameAndPpid(pid, name, &ppid))
        return 0;

    //Common programs that are between terminal and own process, but are not the shell
    if(
        strcasecmp(name, "sh")                   == 0 || //This prevents us from detecting things like pipes and redirects, i hope nobody uses plain `sh` as shell
        strcasecmp(name, "sudo")                 == 0 ||
        strcasecmp(name, "su")                   == 0 ||
        strcasecmp(name, "strace")               == 0 ||
        strcasecmp(name, "sshd")                 == 0 ||
        strcasecmp(name, "gdb")                  == 0 ||
        strcasecmp(name, "lldb")                 == 0 ||
        strcasecmp(name, "lldb-mi")              == 0 ||
        strcasecmp(name, "login")                == 0 ||
        strcasecmp(name, "ltrace")               == 0 ||
        strcasecmp(name, "perf")                 == 0 ||
        strcasecmp(name, "guake-wrapped")        == 0 ||
        strcasestr(name, "debug")             != NULL ||
        strcasestr(name, "command-not-found") != NULL ||
        ffStrEndsWith(name, ".sh")
    )
        return getShellInfo(result, ppid);

    result->shellPid = (uint32_t) pid;
    ffStrbufSetS(&result->shellProcessName, name);
    getProcessInformation(pid, &result->shellProcessName, &result->shellExe, &result->shellExeName);
    return ppid;
}

static pid_t getTerminalInfo(FFTerminalShellResult* result, pid_t pid)
{
    char name[256];
    name[0] = '\0';

    pid_t ppid = 0;

    if(getProcessNameAndPpid(pid, name, &ppid))
        return 0;

    //Known shells
    if (
        strcasecmp(name, "ash")       == 0 ||
        strcasecmp(name, "bash")      == 0 ||
        strcasecmp(name, "zsh")       == 0 ||
        strcasecmp(name, "ksh")       == 0 ||
        strcasecmp(name, "mksh")      == 0 ||
        strcasecmp(name, "oksh")      == 0 ||
        strcasecmp(name, "csh")       == 0 ||
        strcasecmp(name, "tcsh")      == 0 ||
        strcasecmp(name, "fish")      == 0 ||
        strcasecmp(name, "dash")      == 0 ||
        strcasecmp(name, "pwsh")      == 0 ||
        strcasecmp(name, "nu")        == 0 ||
        strcasecmp(name, "git-shell") == 0 ||
        strcasecmp(name, "elvish")    == 0 ||
        strcasecmp(name, "oil.ovm")   == 0 ||
        (strcasecmp(name, "python") == 0 && getenv("XONSH_VERSION"))
    )
        return getTerminalInfo(result, ppid);

    #ifdef __APPLE__
    // https://github.com/fastfetch-cli/fastfetch/discussions/501
    if (ffStrEndsWith(name, " (figterm)") || ffStrEndsWith(name, " (cwterm)"))
        getProcessNameAndPpid(ppid, name, &ppid);
    #endif

    result->terminalPid = (uint32_t) pid;
    ffStrbufSetS(&result->terminalProcessName, name);
    getProcessInformation(pid, &result->terminalProcessName, &result->terminalExe, &result->terminalExeName);
    return ppid;
}

static void getTerminalFromEnv(FFTerminalShellResult* result)
{
    if(
        result->terminalProcessName.length > 0 &&
        !ffStrbufStartsWithIgnCaseS(&result->terminalProcessName, "login") &&
        !ffStrbufIgnCaseEqualS(&result->terminalProcessName, "(login)") &&

        #ifdef __APPLE__
        !ffStrbufIgnCaseEqualS(&result->terminalProcessName, "launchd") &&
        !ffStrbufIgnCaseEqualS(&result->terminalProcessName, "stable") && //for WarpTerminal
        #else
        !ffStrbufIgnCaseEqualS(&result->terminalProcessName, "systemd") &&
        !ffStrbufIgnCaseEqualS(&result->terminalProcessName, "init") &&
        !ffStrbufIgnCaseEqualS(&result->terminalProcessName, "(init)") &&
        #endif

        ffStrbufIgnCaseCompS(&result->terminalProcessName, "0") != 0
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
    if(!ffStrSet(term) || strcasecmp(term, "linux") == 0)
        term = ttyname(STDIN_FILENO);

    if(ffStrSet(term))
    {
        ffStrbufSetS(&result->terminalProcessName, term);
        ffStrbufSetS(&result->terminalExe, term);
        setExeName(&result->terminalExe, &result->terminalExeName);
    }
}

static void getUserShellFromEnv(FFTerminalShellResult* result)
{
    //If shell detection via processes failed
    if(result->shellProcessName.length == 0 && instance.state.platform.userShell.length > 0)
    {
        ffStrbufSet(&result->shellExe, &instance.state.platform.userShell);
        setExeName(&result->shellExe, &result->shellExeName);
        ffStrbufAppendS(&result->shellProcessName, result->shellExeName);
    }
}

bool fftsGetShellVersion(FFstrbuf* exe, const char* exeName, FFstrbuf* version);

bool fftsGetTerminalVersion(FFstrbuf* processName, FFstrbuf* exe, FFstrbuf* version);

static void setShellInfoDetails(FFTerminalShellResult* result)
{
    ffStrbufClear(&result->shellVersion);
    fftsGetShellVersion(&result->shellExe, result->shellExeName, &result->shellVersion);

    if(ffStrbufEqualS(&result->shellProcessName, "pwsh"))
        ffStrbufInitStatic(&result->shellPrettyName, "PowerShell");
    else if(ffStrbufEqualS(&result->shellProcessName, "nu"))
        ffStrbufInitStatic(&result->shellPrettyName, "nushell");
    else if(ffStrbufIgnCaseEqualS(&result->shellProcessName, "python") && getenv("XONSH_VERSION"))
        ffStrbufInitStatic(&result->shellPrettyName, "xonsh");
    else if(ffStrbufIgnCaseEqualS(&result->shellProcessName, "oil.ovm"))
        ffStrbufInitStatic(&result->shellPrettyName, "Oils");
    else
    {
        // https://github.com/fastfetch-cli/fastfetch/discussions/280#discussioncomment-3831734
        ffStrbufInitS(&result->shellPrettyName, result->shellExeName);
    }
}

static void setTerminalInfoDetails(FFTerminalShellResult* result)
{
    if(result->terminalExeName[0] == '.' && ffStrEndsWith(result->terminalExeName, "-wrapped"))
    {
        // For NixOS. Ref: #510 and https://github.com/NixOS/nixpkgs/pull/249428
        // We use terminalProcessName when detecting version and font, overriding it for simplification
        ffStrbufSetNS(
            &result->terminalProcessName,
            (uint32_t) (strlen(result->terminalExeName) - strlen(".-wrapped")),
            result->terminalExeName + 1);
    }

    if(ffStrbufEqualS(&result->terminalProcessName, "wezterm-gui"))
        ffStrbufInitStatic(&result->terminalPrettyName, "WezTerm");
    if(ffStrbufStartsWithS(&result->terminalProcessName, "tmux:"))
        ffStrbufInitStatic(&result->terminalPrettyName, "tmux");

    #if defined(__ANDROID__)

    else if(ffStrbufEqualS(&result->terminalProcessName, "com.termux"))
        ffStrbufInitStatic(&result->terminalPrettyName, "Termux");

    #elif defined(__linux__) || defined(__FreeBSD__)

    else if(ffStrbufStartsWithS(&result->terminalProcessName, "gnome-terminal-"))
        ffStrbufInitStatic(&result->terminalPrettyName, "GNOME Terminal");
    else if(ffStrbufStartsWithS(&result->terminalProcessName, "kgx"))
        ffStrbufInitStatic(&result->terminalPrettyName, "GNOME Console");
    else if(ffStrbufEqualS(&result->terminalProcessName, "urxvt") ||
        ffStrbufEqualS(&result->terminalProcessName, "urxvtd") ||
        ffStrbufEqualS(&result->terminalProcessName, "rxvt")
    )
        ffStrbufInitStatic(&result->terminalPrettyName, "rxvt-unicode");

    #elif defined(__APPLE__)

    else if(ffStrbufEqualS(&result->terminalProcessName, "iTerm.app") || ffStrbufStartsWithS(&result->terminalProcessName, "iTermServer-"))
        ffStrbufInitStatic(&result->terminalPrettyName, "iTerm");
    else if(ffStrbufEqualS(&result->terminalProcessName, "Apple_Terminal"))
        ffStrbufInitStatic(&result->terminalPrettyName, "Apple Terminal");
    else if(ffStrbufEqualS(&result->terminalProcessName, "WarpTerminal"))
        ffStrbufInitStatic(&result->terminalPrettyName, "Warp");

    #endif

    else if(strncmp(result->terminalExeName, result->terminalProcessName.chars, result->terminalProcessName.length) == 0) // if exeName starts with processName, print it. Otherwise print processName
        ffStrbufInitS(&result->terminalPrettyName, result->terminalExeName);
    else
        ffStrbufInitCopy(&result->terminalPrettyName, &result->terminalProcessName);

    ffStrbufInit(&result->terminalVersion);
    fftsGetTerminalVersion(&result->terminalProcessName, &result->terminalExe, &result->terminalVersion);
}

const FFTerminalShellResult* ffDetectTerminalShell()
{
    static FFTerminalShellResult result;
    static bool init = false;
    if(init)
        return &result;
    init = true;

    #ifdef __APPLE__
    const uint32_t exePathLen = PROC_PIDPATHINFO_MAXSIZE;
    #elif defined(MAXPATH)
    const uint32_t exePathLen = MAXPATH;
    #elif defined(PATH_MAX)
    const uint32_t exePathLen = PATH_MAX;
    #else
    const uint32_t exePathLen = 260;
    #endif

    ffStrbufInit(&result.shellProcessName);
    ffStrbufInitA(&result.shellExe, exePathLen);
    result.shellExeName = result.shellExe.chars;
    ffStrbufInit(&result.shellVersion);
    result.shellPid = 0;

    ffStrbufInit(&result.terminalProcessName);
    ffStrbufInitA(&result.terminalExe, exePathLen);
    result.terminalExeName = result.terminalExe.chars;
    result.terminalPid = 0;

    pid_t ppid = getppid();
    ppid = getShellInfo(&result, ppid);
    getUserShellFromEnv(&result);
    setShellInfoDetails(&result);

    if (ppid)
        ppid = getTerminalInfo(&result, ppid);
    getTerminalFromEnv(&result);
    setTerminalInfoDetails(&result);

    return &result;
}
