#include "terminalshell.h"
#include "common/io/io.h"
#include "common/parsing.h"
#include "common/processing.h"
#include "common/thread.h"
#include "util/stringUtils.h"

#include <string.h>
#include <stdlib.h>
#include <unistd.h>

static void setExeName(FFstrbuf* exe, const char** exeName)
{
    assert(exe->length > 0);
    uint32_t lastSlashIndex = ffStrbufLastIndexC(exe, '/');
    if(lastSlashIndex < exe->length)
        *exeName = exe->chars + lastSlashIndex + 1;
}

static pid_t getShellInfo(FFShellResult* result, pid_t pid)
{
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

    while (ffProcessGetBasicInfoLinux(pid, &result->processName, &ppid, &tty) == NULL)
    {
        if (!ffStrbufEqualS(&result->processName, userShellName))
        {
            //Common programs that are between terminal and own process, but are not the shell
            if(
                // tty < 0                                  || //A shell should connect to a tty
                ffStrbufEqualS(&result->processName, "sh")                  || //This prevents us from detecting things like pipes and redirects, i hope nobody uses plain `sh` as shell
                ffStrbufEqualS(&result->processName, "sudo")                ||
                ffStrbufEqualS(&result->processName, "su")                  ||
                ffStrbufEqualS(&result->processName, "strace")              ||
                ffStrbufEqualS(&result->processName, "gdb")                 ||
                ffStrbufEqualS(&result->processName, "lldb")                ||
                ffStrbufEqualS(&result->processName, "lldb-mi")             ||
                ffStrbufEqualS(&result->processName, "login")               ||
                ffStrbufEqualS(&result->processName, "ltrace")              ||
                ffStrbufEqualS(&result->processName, "perf")                ||
                ffStrbufEqualS(&result->processName, "guake-wrapped")       ||
                ffStrbufEqualS(&result->processName, "time")                ||
                ffStrbufContainS(&result->processName, "hyfetch")           || //when hyfetch uses fastfetch as backend
                ffStrbufEqualS(&result->processName, "clifm")               || //https://github.com/leo-arch/clifm/issues/289
                ffStrbufEqualS(&result->processName, "valgrind")            ||
                ffStrbufEqualS(&result->processName, "fastfetch")           || //994
                ffStrbufEqualS(&result->processName, "flashfetch")          ||
                ffStrbufContainS(&result->processName, "debug")             ||
                ffStrbufContainS(&result->processName, "not-found")         ||
                ffStrbufEndsWithS(&result->processName, ".sh")
            )
            {
                pid = ppid;
                ffStrbufClear(&result->processName);
                continue;
            }
        }

        result->pid = (uint32_t) pid;
        result->ppid = (uint32_t) ppid;
        result->tty = tty;
        ffProcessGetInfoLinux(pid, &result->processName, &result->exe, &result->exeName, &result->exePath);
        break;
    }
    return ppid;
}

static pid_t getTerminalInfo(FFTerminalResult* result, pid_t pid)
{
    pid_t ppid = 0;

    while (ffProcessGetBasicInfoLinux(pid, &result->processName, &ppid, NULL) == NULL)
    {
        //Known shells
        if (
            ffStrbufEqualS(&result->processName, "sh")         ||
            ffStrbufEqualS(&result->processName, "ash")        ||
            ffStrbufEqualS(&result->processName, "bash")       ||
            ffStrbufEqualS(&result->processName, "zsh")        ||
            ffStrbufEqualS(&result->processName, "ksh")        ||
            ffStrbufEqualS(&result->processName, "mksh")       ||
            ffStrbufEqualS(&result->processName, "oksh")       ||
            ffStrbufEqualS(&result->processName, "csh")        ||
            ffStrbufEqualS(&result->processName, "tcsh")       ||
            ffStrbufEqualS(&result->processName, "fish")       ||
            ffStrbufEqualS(&result->processName, "dash")       ||
            ffStrbufEqualS(&result->processName, "pwsh")       ||
            ffStrbufEqualS(&result->processName, "nu")         ||
            ffStrbufEqualS(&result->processName, "git-shell")  ||
            ffStrbufEqualS(&result->processName, "elvish")     ||
            ffStrbufEqualS(&result->processName, "oil.ovm")    ||
            ffStrbufEqualS(&result->processName, "xonsh")      || // works in Linux but not in macOS because kernel returns `Python` in this case
            ffStrbufEqualS(&result->processName, "login")      ||
            ffStrbufEqualS(&result->processName, "clifm")      || // https://github.com/leo-arch/clifm/issues/289
            ffStrbufEqualS(&result->processName, "chezmoi")    || // #762
            #ifdef __linux__
            ffStrbufStartsWithS(&result->processName, "flatpak-") || // #707
            #endif
            ffStrbufEndsWithS(&result->processName, ".sh")
        )
        {
            pid = ppid;
            ffStrbufClear(&result->processName);
            continue;
        }

        #ifdef __APPLE__
        // https://github.com/fastfetch-cli/fastfetch/discussions/501
        const char* pLeft = strstr(result->processName.chars, " (");
        if (pLeft)
        {
            pLeft += 2;
            const char* pRight = strstr(pLeft, "term)");
            if (pRight && pRight[5] == '\0')
            {
                for (; pLeft < pRight; ++pLeft)
                    if (*pLeft < 'a' || *pLeft > 'z')
                        break;
                if (pLeft == pRight && ffProcessGetBasicInfoLinux(ppid, &result->processName, &ppid, NULL) != NULL)
                    return 0;
            }
        }
        #endif

        result->pid = (uint32_t) pid;
        result->ppid = (uint32_t) ppid;
        ffProcessGetInfoLinux(pid, &result->processName, &result->exe, &result->exeName, &result->exePath);
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
    if (ffProcessGetBasicInfoLinux(pid, &result->processName, (pid_t*) &result->ppid, NULL) == NULL)
    {
        ffProcessGetInfoLinux(pid, &result->processName, &result->exe, &result->exeName, &result->exePath);
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

bool fftsGetShellVersion(FFstrbuf* exe, const char* exeName, FFstrbuf* exePath, FFstrbuf* version);

bool fftsGetTerminalVersion(FFstrbuf* processName, FFstrbuf* exe, FFstrbuf* version);

static void setShellInfoDetails(FFShellResult* result)
{
    ffStrbufClear(&result->version);
    fftsGetShellVersion(&result->exe, result->exeName, &result->exePath, &result->version);

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
    else if(ffStrbufEqualS(&result->processName, "sshd") || ffStrbufStartsWithS(&result->processName, "sshd-"))
        ffStrbufInitCopy(&result->prettyName, &result->tty);

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
    else if(ffStrbufStartsWithS(&result->processName, "ptyxis-agent"))
        ffStrbufInitStatic(&result->prettyName, "Ptyxis");

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

    const char* ignoreParent = getenv("FFTS_IGNORE_PARENT");
    if (ignoreParent && ffStrEquals(ignoreParent, "1"))
    {
        FF_STRBUF_AUTO_DESTROY _ = ffStrbufCreate();
        ffProcessGetBasicInfoLinux(ppid, &_, &ppid, NULL);
    }

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
