#include "terminalshell.h"
#include "common/io/io.h"
#include "common/parsing.h"
#include "common/processing.h"
#include "common/thread.h"
#include "util/stringUtils.h"

#include <ctype.h>
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
        ffStrbufSubstrBeforeFirstC(exe, '\0'); //Trim the arguments
        ffStrbufTrimLeft(exe, '-'); //Happens in TTY
    }

    #elif defined(__APPLE__)

    int length = proc_pidpath((int)pid, exe->chars, exe->allocated);
    if(length > 0)
        exe->length = (uint32_t)length;

    #else

    size_t size = exe->allocated;
    if(!sysctl(
        (int[]){CTL_KERN, KERN_PROC, KERN_PROC_PATHNAME, pid}, 4,
        exe->chars, &size,
        NULL, 0
    ))
        exe->length = (uint32_t)size;

    #endif

    if(exe->length == 0)
        ffStrbufSet(exe, processName);

    setExeName(exe, exeName);
}

static const char* getProcessNameAndPpid(pid_t pid, char* name, pid_t* ppid)
{
    const char* error = NULL;

    #ifdef __linux__

    char statFilePath[64];
    snprintf(statFilePath, sizeof(statFilePath), "/proc/%d/stat", (int)pid);
    FILE* stat = fopen(statFilePath, "r");
    if(stat == NULL)
        return "fopen(statFilePath, \"r\") failed";

    *ppid = 0;
    if(
        fscanf(stat, "%*s (%255[^)]) %*c %d", name, ppid) != 2 || //stat (comm) state ppid
        !ffStrSet(name) ||
        *ppid == 0
    )
        error = "fscanf(stat) failed";

    fclose(stat);

    #elif defined(__APPLE__)

    struct proc_bsdshortinfo proc;
    if(proc_pidinfo(pid, PROC_PIDT_SHORTBSDINFO, 0, &proc, PROC_PIDT_SHORTBSDINFO_SIZE) <= 0)
        error = "proc_pidinfo(pid) failed";
    else
    {
        *ppid = (pid_t)proc.pbsi_ppid;
        strncpy(name, proc.pbsi_comm, 16); //trancated to 16 chars
    }

    #else

    struct kinfo_proc proc;
    size_t size = sizeof(proc);
    if(sysctl(
        (int[]){CTL_KERN, KERN_PROC, KERN_PROC_PID, pid}, 4,
        &proc, &size,
        NULL, 0
    ))
        error = "sysctl(KERN_PROC_PID) failed";
    else
    {
        *ppid = (pid_t)proc.ki_ppid;
        strncpy(name, proc.ki_comm, COMMLEN);
    }

    #endif

    return error;
}

static void getTerminalShell(FFTerminalShellResult* result, pid_t pid)
{
    char name[256];
    name[0] = '\0';

    pid_t ppid = 0;

    if(getProcessNameAndPpid(pid, name, &ppid))
        return;

    //Common programs that are between terminal and own process, but are not the shell
    if(
        strcasecmp(name, "sh")                   == 0 || //This prevents us from detecting things like pipes and redirects, i hope nobody uses plain `sh` as shell
        strcasecmp(name, "sudo")                 == 0 ||
        strcasecmp(name, "su")                   == 0 ||
        strcasecmp(name, "strace")               == 0 ||
        strcasecmp(name, "sshd")                 == 0 ||
        strcasecmp(name, "gdb")                  == 0 ||
        strcasecmp(name, "lldb")                 == 0 ||
        strcasecmp(name, "guake-wrapped")        == 0 ||
        strcasestr(name, "debug")             != NULL ||
        strcasestr(name, "command-not-found") != NULL ||
        ffStrEndsWith(name, ".sh")
    ) {
        getTerminalShell(result, ppid);
        return;
    }

    //Known shells
    if (
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
        (strcasecmp(name, "python") == 0 && getenv("XONSH_VERSION"))
    ) {
        if (result->shellProcessName.length == 0)
        {
            result->shellPid = (uint32_t) pid;
            ffStrbufSetS(&result->shellProcessName, name);
            getProcessInformation(pid, &result->shellProcessName, &result->shellExe, &result->shellExeName);
        }

        getTerminalShell(result, ppid);
        return;
    }

    #ifdef __APPLE__
    // https://github.com/fastfetch-cli/fastfetch/discussions/501
    if (ffStrEndsWith(name, " (figterm)"))
        getProcessNameAndPpid(ppid, name, &ppid);
    #endif

    result->terminalPid = (uint32_t) pid;
    ffStrbufSetS(&result->terminalProcessName, name);
    getProcessInformation(pid, &result->terminalProcessName, &result->terminalExe, &result->terminalExeName);
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
        getenv("TERMUX_MAIN_PACKAGE_FORMAT") != NULL ||
        getenv("TMUX_TMPDIR") != NULL
    )) term = "Termux";
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
    ffStrbufSet(&result->userShellExe, &instance.state.platform.userShell);
    if(result->userShellExe.length == 0)
        return;

    setExeName(&result->userShellExe, &result->userShellExeName);

    //If shell detection via processes failed
    if(result->shellProcessName.length == 0 && result->userShellExe.length > 0)
    {
        ffStrbufAppendS(&result->shellProcessName, result->userShellExeName);
        ffStrbufSet(&result->shellExe, &result->userShellExe);
        setExeName(&result->shellExe, &result->shellExeName);
    }
}

static void getShellVersionGeneric(FFstrbuf* exe, const char* exeName, FFstrbuf* version)
{
    FF_STRBUF_AUTO_DESTROY command = ffStrbufCreate();
    ffStrbufAppendS(&command, "printf \"%s\" \"$");
    ffStrbufAppendTransformS(&command, exeName, toupper);
    ffStrbufAppendS(&command, "_VERSION\"");

    ffProcessAppendStdOut(version, (char* const[]) {
        "env",
        "-i",
        exe->chars,
        "-c",
        command.chars,
        NULL
    });
    ffStrbufSubstrBeforeFirstC(version, '(');
    ffStrbufRemoveStrings(version, 2, (const char*[]) { "-release", "release" });
}

bool fftsGetShellVersion(FFstrbuf* exe, const char* exeName, FFstrbuf* version);

static void getShellVersion(FFstrbuf* exe, const char* exeName, FFstrbuf* version)
{
    ffStrbufClear(version);
    if(!fftsGetShellVersion(exe, exeName, version))
        getShellVersionGeneric(exe, exeName, version);
}

bool fftsGetTerminalVersion(FFstrbuf* processName, FFstrbuf* exe, FFstrbuf* version);

const FFTerminalShellResult* ffDetectTerminalShell()
{
    static FFThreadMutex mutex = FF_THREAD_MUTEX_INITIALIZER;
    static FFTerminalShellResult result;
    static bool init = false;
    ffThreadMutexLock(&mutex);
    if(init)
    {
        ffThreadMutexUnlock(&mutex);
        return &result;
    }
    init = true;

    #ifdef __APPLE__
    const uint32_t exePathLen = PROC_PIDPATHINFO_MAXSIZE;
    #elif defined(MAXPATH)
    const uint32_t exePathLen = MAXPATH;
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

    ffStrbufInit(&result.userShellExe);
    result.userShellExeName = result.userShellExe.chars;
    ffStrbufInit(&result.userShellVersion);

    getTerminalShell(&result, getppid());

    getTerminalFromEnv(&result);
    getUserShellFromEnv(&result);

    ffStrbufClear(&result.shellVersion);
    getShellVersion(&result.shellExe, result.shellExeName, &result.shellVersion);

    if(strcasecmp(result.shellExeName, result.userShellExeName) != 0)
        getShellVersion(&result.userShellExe, result.userShellExeName, &result.userShellVersion);
    else
        ffStrbufSet(&result.userShellVersion, &result.shellVersion);

    if(ffStrbufEqualS(&result.shellProcessName, "pwsh"))
        ffStrbufInitStatic(&result.shellPrettyName, "PowerShell");
    else if(ffStrbufEqualS(&result.shellProcessName, "nu"))
        ffStrbufInitStatic(&result.shellPrettyName, "nushell");
    else if(ffStrbufIgnCaseEqualS(&result.shellProcessName, "python") && getenv("XONSH_VERSION"))
        ffStrbufInitStatic(&result.shellPrettyName, "xonsh");
    else
    {
        // https://github.com/fastfetch-cli/fastfetch/discussions/280#discussioncomment-3831734
        ffStrbufInitS(&result.shellPrettyName, result.shellExeName);
    }

    if(result.terminalExeName[0] == '.' && ffStrEndsWith(result.terminalExeName, "-wrapper"))
    {
        // For NixOS. Ref: #510 and https://github.com/NixOS/nixpkgs/pull/249428
        // We use terminalProcessName when detecting version and font, overriding it for simplication
        ffStrbufSetNS(
            &result.terminalProcessName,
            (uint32_t) (strlen(result.terminalExeName) - strlen(".-wrapper")),
            result.terminalExeName + 1);
    }


    if(ffStrbufEqualS(&result.terminalProcessName, "wezterm-gui"))
        ffStrbufInitStatic(&result.terminalPrettyName, "WezTerm");
    #if defined(__linux__) || defined(__FreeBSD__)

    else if(ffStrbufStartsWithS(&result.terminalProcessName, "gnome-terminal-"))
        ffStrbufInitStatic(&result.terminalPrettyName, "gnome-terminal");

    #elif defined(__APPLE__)

    else if(ffStrbufEqualS(&result.terminalProcessName, "iTerm.app") || ffStrbufStartsWithS(&result.terminalProcessName, "iTermServer-"))
        ffStrbufInitStatic(&result.terminalPrettyName, "iTerm");
    else if(ffStrbufEqualS(&result.terminalProcessName, "Apple_Terminal"))
        ffStrbufInitStatic(&result.terminalPrettyName, "Apple Terminal");
    else if(ffStrbufEqualS(&result.terminalProcessName, "WarpTerminal"))
        ffStrbufInitStatic(&result.terminalPrettyName, "Warp");

    #endif

    else if(strncmp(result.terminalExeName, result.terminalProcessName.chars, result.terminalProcessName.length) == 0) // if exeName starts with processName, print it. Otherwise print processName
        ffStrbufInitS(&result.terminalPrettyName, result.terminalExeName);
    else
        ffStrbufInitCopy(&result.terminalPrettyName, &result.terminalProcessName);

    ffStrbufInit(&result.terminalVersion);
    fftsGetTerminalVersion(&result.terminalProcessName, &result.terminalExe, &result.terminalVersion);

    ffThreadMutexUnlock(&mutex);
    return &result;
}
