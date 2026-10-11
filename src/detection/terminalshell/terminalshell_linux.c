#include "terminalshell.h"
#include "common/FFcache.h"
#include "common/io.h"
#include "common/parsing.h"
#include "common/processing.h"
#include "common/thread.h"
#include "common/strutil.h"

#include <string.h>
#include <stdlib.h>
#include <unistd.h>

static void setExeName(FFstrbuf* exe, const char** exeName) {
    assert(exe->length > 0);
    uint32_t lastSlashIndex = ffStrbufLastIndexC(exe, '/');
    if (lastSlashIndex < exe->length) {
        *exeName = exe->chars + lastSlashIndex + 1;
    }
}

static pid_t getShellInfo(FFShellResult* result, pid_t pid) {
    pid_t ppid = 0;
    int32_t tty = -1;

    const char* userShellName = nullptr;
    {
        uint32_t index = ffStrbufLastIndexC(&instance.state.platform.userShell, '/');
        if (index == instance.state.platform.userShell.length) {
            userShellName = instance.state.platform.userShell.chars;
        } else {
            userShellName = instance.state.platform.userShell.chars + index + 1;
        }
    }

    while (pid > 1 && ffProcessGetBasicInfoLinux(pid, &result->processName, &ppid, &tty) == nullptr) {
        if (!ffStrbufEqualS(&result->processName, userShellName)) {
            // Common programs that are between terminal and own process, but are not the shell
            if (
                // tty < 0                                  || //A shell should connect to a tty
                pid == 1 ||                                   // init/systemd
                ffStrbufEqualS(&result->processName, "sh") || // This prevents us from detecting things like pipes and redirects, i hope nobody uses plain `sh` as shell
                ffStrbufEqualS(&result->processName, "sudo") ||
                ffStrbufEqualS(&result->processName, "su") ||
                ffStrbufEqualS(&result->processName, "strace") ||
                ffStrbufEqualS(&result->processName, "gdb") ||
                ffStrbufEqualS(&result->processName, "lldb") ||
                ffStrbufEqualS(&result->processName, "lldb-mi") ||
                ffStrbufEqualS(&result->processName, "login") ||
                ffStrbufEqualS(&result->processName, "ltrace") ||
                ffStrbufEqualS(&result->processName, "perf") ||
                ffStrbufEqualS(&result->processName, "guake-wrapped") ||
                ffStrbufEqualS(&result->processName, "time") ||
                ffStrbufEqualS(&result->processName, "clifm") || // https://github.com/leo-arch/clifm/issues/289
                ffStrbufEqualS(&result->processName, "valgrind") ||
                ffStrbufEqualS(&result->processName, "fastfetch") || // #994
                ffStrbufEqualS(&result->processName, "flashfetch") ||
                ffStrbufEqualS(&result->processName, "proot") ||
                ffStrbufEqualS(&result->processName, "script") ||
#ifdef __linux__
                ffStrbufEqualS(&result->processName, "run-parts") ||
#endif
                ffStrbufContainS(&result->processName, "debug") ||
                ffStrbufContainS(&result->processName, "command-not-") ||
                ffStrbufEndsWithS(&result->processName, ".sh")) {
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
    return pid > 1 ? ppid : 0;
}

/* 3 functions for examining Emacs state & provenance*/
static bool isEmacsExecutable(const char* name)
{
    /*
     * Linux may expose Emacs's process comm as ".emacs-31.1-rea"
     * (truncated to TASK_COMM_LEN), while exeName may be NULL and
     * exe may report "emacs".
     */
    return name != nullptr &&
        (strcmp(name, "emacs") == 0 ||
         strncmp(name, "emacs-", 6) == 0 ||
         strncmp(name, ".emacs-", 7) == 0); // to catch Guix/Nix bins
}

static bool isEmacsTextMode(pid_t pid)
{
    if (pid <= 1)
        return false;

    char path[64];
    snprintf(path, sizeof(path), "/proc/%ld/cmdline", (long) pid);

    FILE* file = fopen(path, "r");
    if (file == nullptr)
        return false;

    char arg[256];
    size_t len = 0;
    bool overflow = false;
    bool textMode = false;
    int ch;

    while ((ch = fgetc(file)) != EOF) {
        if (ch == '\0') {
            if (!overflow) {
                arg[len] = '\0';

                if (strcmp(arg, "-nw") == 0 ||
                    strcmp(arg, "--no-window-system") == 0 ||
                    strcmp(arg, "-t") == 0 ||
                    strcmp(arg, "--tty") == 0 ||
                    strncmp(arg, "--terminal=", 11) == 0) {
                    textMode = true;
                    break;
                }
            }

            len = 0;
            overflow = false;
        } else if (len < sizeof(arg) - 1) {
            arg[len++] = (char) ch;
        } else {
            overflow = true;
        }
    }

    if (!textMode && !overflow && len > 0) {
        arg[len] = '\0';

        textMode =
            strcmp(arg, "-nw") == 0 ||
            strcmp(arg, "--no-window-system") == 0 ||
            strcmp(arg, "-t") == 0 ||
            strcmp(arg, "--tty") == 0 ||
            strncmp(arg, "--terminal=", 11) == 0;
    }

    fclose(file);
    return textMode;
}

static pid_t findEmacsAncestor(pid_t pid)
{
    FFstrbuf processName, exe, exePath;

    ffStrbufInit(&processName);
    ffStrbufInit(&exe);
    ffStrbufInit(&exePath);

    pid_t emacsPid = 0;

    for (unsigned depth = 0; pid > 1 && depth < 256; ++depth) {
        pid_t ppid = 0;
        const char* exeName = nullptr;

        ffStrbufClear(&processName);
        ffStrbufClear(&exe);
        ffStrbufClear(&exePath);

        if (ffProcessGetBasicInfoLinux(
                pid, &processName, &ppid, nullptr) != nullptr)
            break;

        ffProcessGetInfoLinux(
            pid, &processName, &exe, &exeName, &exePath);

        const char* exePathBase = strrchr(exePath.chars, '/');
        exePathBase = exePathBase != nullptr
            ? exePathBase + 1
            : exePath.chars;

        if (isEmacsExecutable(exeName) ||
            isEmacsExecutable(exe.chars) ||
            isEmacsExecutable(exePathBase) ||
            isEmacsExecutable(processName.chars)) {
            emacsPid = pid;
            break;
        }

        if (ppid <= 0 || ppid == pid)
            break;

        pid = ppid;
    }

    ffStrbufDestroy(&processName);
    ffStrbufDestroy(&exe);
    ffStrbufDestroy(&exePath);

    return emacsPid;
}

/* for tracing provenance, esp. for Emacs-related things*/
static bool isProcessAncestor(pid_t ancestor, pid_t descendant)
{
    if (ancestor <= 1 || descendant <= 1)
        return false;

    FF_STRBUF_AUTO_DESTROY processName = ffStrbufCreate();

    for (unsigned depth = 0; descendant > 1 && depth < 256; ++depth) {
        pid_t ppid = 0;
        ffStrbufClear(&processName);

        if (ffProcessGetBasicInfoLinux(
                descendant, &processName, &ppid, nullptr) != nullptr)
            return false;

        if (descendant == ancestor)
            return true;

        if (ppid <= 0 || ppid == descendant)
            return false;

        descendant = ppid;
    }

    return false;
}

static pid_t getTerminalInfo(FFTerminalResult* result, pid_t pid, bool skipEmacs) {
    // skipEmacs lets us only skip things when tracing emacs provenance
    pid_t ppid = 0;

    // get detected shell
    const FFShellResult* shell = ffDetectShell();

    while (pid > 1 && ffProcessGetBasicInfoLinux(pid, &result->processName, &ppid, nullptr) == nullptr) {
        // Known shells
        if (
            pid == 1 || // init/systemd
            ffStrbufEqualS(&result->processName, "sudo") ||
            ffStrbufEqualS(&result->processName, "su") ||
            ffStrbufEqualS(&result->processName, "sh") ||
            ffStrbufEqualS(&result->processName, "ash") ||
            ffStrbufEqualS(&result->processName, "bash") ||
            ffStrbufEqualS(&result->processName, "zsh") ||
            ffStrbufEqualS(&result->processName, "ksh") ||
            ffStrbufEqualS(&result->processName, "ksh93") ||
            ffStrbufEqualS(&result->processName, "mksh") ||
            ffStrbufEqualS(&result->processName, "pdksh") ||
            ffStrbufEqualS(&result->processName, "oksh") ||
            ffStrbufEqualS(&result->processName, "csh") ||
            ffStrbufEqualS(&result->processName, "tcsh") ||
            ffStrbufEqualS(&result->processName, "fish") ||
            ffStrbufEqualS(&result->processName, "dash") ||
            ffStrbufEqualS(&result->processName, "pwsh") ||
            ffStrbufEqualS(&result->processName, "nu") ||
            ffStrbufEqualS(&result->processName, "git-shell") ||
            ffStrbufEqualS(&result->processName, "elvish") ||
            ffStrbufEqualS(&result->processName, "oil.ovm") ||
            ffStrbufEqualS(&result->processName, "xonsh") || // works in Linux but not in macOS because kernel returns `Python` in this case
            ffStrbufEqualS(&result->processName, "login") ||
            ffStrbufEqualS(&result->processName, "clifm") ||   // https://github.com/leo-arch/clifm/issues/289
            ffStrbufEqualS(&result->processName, "chezmoi") || // #762
            (skipEmacs && shell->processName.length > 0 &&     // skip over something registered as shell...
             ffStrbufEqual(&result->processName, &shell->processName)) ||  // ...for now restricted to Emacs conditions (could useful generally?)
            ffStrbufEqualS(&result->processName, "proot") ||
            ffStrbufEqualS(&result->processName, "script") ||
            (skipEmacs && ffStrbufEqualS(&result->processName, "guix")) || // add "nix"?
            (skipEmacs && ffStrbufEqualS(&result->processName, "fastfetch")) ||  // skip utilities that sit between the shell and ...
            (skipEmacs && ffStrbufEqualS(&result->processName, "flashfetch")) || // the program being run
            (skipEmacs && isEmacsExecutable(result->processName.chars)) || // only skip when specifically tracing emacs ancestry
            /* isEmacsExecutable(result->processName.chars) || */
#ifdef __linux__
            ffStrbufStartsWithS(&result->processName, "Relay(") ||   // Unknown process in WSL2
            ffStrbufStartsWithS(&result->processName, "flatpak-") || // #707
            ffStrbufEqualS(&result->processName, "run-parts") ||     // #2048
#endif
            ffStrbufEndsWithS(&result->processName, ".sh")) {
            pid = ppid;
            ffStrbufClear(&result->processName);
            continue;
        }

#ifdef __APPLE__
        // https://github.com/fastfetch-cli/fastfetch/discussions/501
        const char* pLeft = strstr(result->processName.chars, " (");
        if (pLeft) {
            pLeft += 2;
            const char* pRight = strstr(pLeft, "term)");
            if (pRight && pRight[5] == '\0') {
                for (; pLeft < pRight; ++pLeft) {
                    if (*pLeft < 'a' || *pLeft > 'z') {
                        break;
                    }
                }
                if (pLeft == pRight && ffProcessGetBasicInfoLinux(ppid, &result->processName, &ppid, nullptr) != nullptr) {
                    return 0;
                }
            }
        }
#endif

        result->pid = (uint32_t) pid;
        result->ppid = (uint32_t) ppid;
        ffProcessGetInfoLinux(pid, &result->processName, &result->exe, &result->exeName, &result->exePath);
        break;
    }
    return pid > 1 ? ppid : 0;
}

static bool getTerminalInfoByPidEnv(FFTerminalResult* result, const char* pidEnv) {
    const char* envStr = getenv(pidEnv);
    if (envStr == nullptr) {
        return false;
    }

    pid_t pid = (pid_t) strtol(envStr, nullptr, 10);
    result->pid = (uint32_t) pid;
    if (ffProcessGetBasicInfoLinux(pid, &result->processName, (pid_t*) &result->ppid, nullptr) == nullptr) {
        ffProcessGetInfoLinux(pid, &result->processName, &result->exe, &result->exeName, &result->exePath);
        return true;
    }

    return false;
}

static const char* getEmacsTerminalNameHint(void) {
    const char* insideEmacs = getenv("INSIDE_EMACS");
    if (insideEmacs == nullptr) {
        return nullptr;
    }

    const char* frontend = strrchr(insideEmacs, ',');
    frontend = frontend ? frontend + 1 : insideEmacs;

    size_t len = strcspn(frontend, ":");

    if (len == 0) {
        return nullptr;
    }
    if (len == 6 && strncmp(frontend, "comint", len) == 0) {
        return "comint";
    }
    if (len == 4 && strncmp(frontend, "term", len) == 0) {
        return "term/ansi-term";
    }
    if (len == 6 && strncmp(frontend, "eshell", len) == 0) {
        return "eshell";
    }
    if (len == 5 && strncmp(frontend, "vterm", len) == 0) {
        return "vterm";
    }
    if (len == 3 && strncmp(frontend, "eat", len) == 0) {
        return "eat";
    }
    if (len == 7 && strncmp(frontend, "ghostel", len) == 0) {
        return "ghostel";
    }

    return nullptr;
}

static void getTerminalFromEnv(FFTerminalResult* result) {
    if (result->processName.length > 0) {
        if (!ffStrbufStartsWithS(&result->processName, "login") &&
            !ffStrbufEqualS(&result->processName, "(login)") &&

#ifdef __APPLE__
            !ffStrbufEqualS(&result->processName, "launchd") &&
#else
            !ffStrbufEqualS(&result->processName, "systemd") &&
            !ffStrbufEqualS(&result->processName, "init") &&
            !ffStrbufEqualS(&result->processName, "(init)") &&
            !ffStrbufEqualS(&result->processName, "SessionLeader") && // #750
#endif

            !ffStrbufEqualS(&result->processName, "0"))
            return;

        ffStrbufClear(&result->processName);
        ffStrbufClear(&result->exe);
        result->exeName = result->exe.chars;
        ffStrbufClear(&result->exePath);
        result->pid = result->ppid = 0;
    }

    const char* term = nullptr;

    // SSH
    if (
        getenv("SSH_TTY") != nullptr) {
        term = getenv("SSH_TTY");
    } else if (
        getenv("KITTY_PID") != nullptr ||
        getenv("KITTY_INSTALLATION_DIR") != nullptr) {
        if (getTerminalInfoByPidEnv(result, "KITTY_PID")) {
            return;
        }
        term = "kitty";
    }

#ifdef __linux__ // WSL
    // Windows Terminal
    else if (
        getenv("WT_SESSION") != nullptr ||
        getenv("WT_PROFILE_ID") != nullptr)
        term = "Windows Terminal";

    // ConEmu
    else if (
        getenv("ConEmuPID") != nullptr) {
        term = "ConEmu";
    }
#endif

    // Alacritty
    else if (
        getenv("ALACRITTY_SOCKET") != nullptr ||
        getenv("ALACRITTY_LOG") != nullptr ||
        getenv("ALACRITTY_WINDOW_ID") != nullptr) {
        term = "Alacritty";
    }
#ifdef __ANDROID__
    // Termux
    else if (
        getenv("TERMUX_VERSION") != nullptr ||
        getenv("TERMUX_MAIN_PACKAGE_FORMAT") != nullptr) {
        if (getTerminalInfoByPidEnv(result, "TERMUX_APP__PID")) {
            return;
        }
        term = "com.termux";
    }
#endif

#if defined(__linux__) || defined(__FreeBSD__) || defined(__NetBSD__) || defined(__GNU__)
    // Konsole
    else if (
        getenv("KONSOLE_VERSION") != nullptr)
        term = "konsole";

    else if (
        getenv("GNOME_TERMINAL_SCREEN") != nullptr ||
        getenv("GNOME_TERMINAL_SERVICE") != nullptr) {
        term = "gnome-terminal";
    }
#endif

    // MacOS, mintty
    else if (getenv("TERM_PROGRAM") != nullptr) {
        term = getenv("TERM_PROGRAM");
    }

    else if (getenv("LC_TERMINAL") != nullptr) {
        term = getenv("LC_TERMINAL");
    }

    // Normal Terminal
    else {
        term = getenv("TERM");
        // TTY
        if (!ffStrSet(term) || ffStrEquals(term, "linux")) {
            term = ttyname(STDIN_FILENO);
        }
    }

    if (ffStrSet(term)) {
        ffStrbufSetS(&result->processName, term);
        ffStrbufSetS(&result->exe, term);
        setExeName(&result->exe, &result->exeName);
    }
}

static void getUserShellFromEnv(FFShellResult* result) {
    // If shell detection via processes failed
    if (result->processName.length == 0 && instance.state.platform.userShell.length > 0) {
        ffStrbufSet(&result->exe, &instance.state.platform.userShell);
        setExeName(&result->exe, &result->exeName);
        ffStrbufAppendS(&result->processName, result->exeName);
    }
}

static bool isEmacsEshell(void) {
    const char* insideEmacs = getenv("INSIDE_EMACS");
    if (insideEmacs == nullptr) {
        return false;
    }

    const char* frontend = strrchr(insideEmacs, ',');
    frontend = frontend ? frontend + 1 : insideEmacs;

    return strncmp(frontend, "eshell", 6) == 0 &&
        (frontend[6] == '\0' || frontend[6] == ':');
}

static void setShellInfoDetails(FFShellResult* result) {
    if (ffStrbufEqualS(&result->processName, "pwsh")) {
        ffStrbufInitStatic(&result->prettyName, "PowerShell");
    } else if (ffStrbufEqualS(&result->processName, "nu")) {
        ffStrbufInitStatic(&result->prettyName, "nushell");
    } else if (ffStrbufEqualS(&result->processName, "oil.ovm")) {
        ffStrbufInitStatic(&result->prettyName, "Oils");
    } else if (ffStrbufEqualS(&result->processName, "busybox")) {
        ffStrbufInitStatic(&result->prettyName, "ash");
    } else {
        // https://github.com/fastfetch-cli/fastfetch/discussions/280#discussioncomment-3831734
        ffStrbufInitS(&result->prettyName, result->exeName);
    }
}

static void setTerminalInfoDetails(FFTerminalResult* result) {
    // For Nixpkgs. Ref: #510 and https://github.com/NixOS/nixpkgs/pull/249428
    // We use processName when detecting version and font, overriding it for simplification
    if (ffStrbufStartsWithC(&result->processName, '.') && ffStrbufStartsWithS(&result->exePath,"/nix/store")) {
        ffStrbufSubstrAfter(&result->processName, 0);
        if (strlen(result->exeName) < 15) {
            ffStrbufSubstrBeforeLastC(&result->processName, '-');
        }
    }

    if (ffStrbufEqualS(&result->processName, "wezterm-gui")) {
        ffStrbufInitStatic(&result->prettyName, "WezTerm");
    } else if (ffStrbufStartsWithS(&result->processName, "tmux:")) {
        ffStrbufInitStatic(&result->prettyName, "tmux");
    } else if (ffStrbufStartsWithS(&result->processName, "screen-")) {
        ffStrbufInitStatic(&result->prettyName, "screen");
    } else if (ffStrbufEqualS(&result->processName, "sshd") || ffStrbufStartsWithS(&result->processName, "sshd-")) {
        if (result->tty.length) {
            ffStrbufInitCopy(&result->prettyName, &result->tty);
        } else {
            ffStrbufSetStatic(&result->prettyName, "sshd");
        }
    }

#if defined(__ANDROID__)

    else if (ffStrbufEqualS(&result->processName, "com.termux"))
        ffStrbufInitStatic(&result->prettyName, "Termux");

#elif defined(__linux__) || defined(__FreeBSD__) || defined(__NetBSD__) || defined(__GNU__)

    else if (ffStrbufStartsWithS(&result->processName, "gnome-terminal"))
        ffStrbufInitStatic(&result->prettyName, "GNOME Terminal");
    else if (ffStrbufStartsWithS(&result->processName, "kgx")) {
        ffStrbufInitStatic(&result->prettyName, "GNOME Console");
    } else if (ffStrbufEqualS(&result->processName, "urxvt") ||
        ffStrbufEqualS(&result->processName, "urxvtd") ||
        ffStrbufEqualS(&result->processName, "rxvt")) {
        ffStrbufInitStatic(&result->prettyName, "rxvt-unicode");
    } else if (ffStrbufStartsWithS(&result->processName, "ptyxis-agent")) {
        ffStrbufInitStatic(&result->prettyName, "Ptyxis");
    }

#elif defined(__APPLE__)

    else if (ffStrbufEqualS(&result->processName, "iTerm.app") || ffStrbufStartsWithS(&result->processName, "iTermServer-"))
        ffStrbufInitStatic(&result->prettyName, "iTerm");
    else if (ffStrbufEndsWithS(&result->exePath, "Terminal.app/Contents/MacOS/Terminal")) {
        ffStrbufSetStatic(&result->processName, "Apple_Terminal"); // $TERM_PROGRAM, for terminal font detection
        ffStrbufInitStatic(&result->prettyName, "Apple Terminal");
    } else if (ffStrbufEqualS(&result->processName, "Apple_Terminal")) {
        ffStrbufInitStatic(&result->prettyName, "Apple Terminal");
    } else if (ffStrbufEndsWithS(&result->exePath, "Warp.app/Contents/MacOS/stable")) {
        ffStrbufSetStatic(&result->processName, "WarpTerminal"); // $TERM_PROGRAM, for terminal font detection
        ffStrbufInitStatic(&result->prettyName, "Warp");
    } else if (ffStrbufEqualS(&result->processName, "WarpTerminal")) {
        ffStrbufInitStatic(&result->prettyName, "Warp");
    }

#elif defined(__HAIKU__)

    else if (ffStrbufEqualS(&result->processName, "Terminal"))
        ffStrbufInitStatic(&result->prettyName, "Haiku Terminal");

#endif

    else if (strncmp(result->exeName, result->processName.chars, result->processName.length) == 0 || (ffStrbufStartsWithS(&result->exePath,"/nix/store") && strlen(result->exeName) > 15)) { // if exeName starts with processName, print it. Otherwise print processName. For nixpkgs, use exeName if processName can't be unwrapped
        ffStrbufInitS(&result->prettyName, result->exeName);
    } else {
        ffStrbufInitCopy(&result->prettyName, &result->processName);
    }
}

#if defined(MAXPATH)
    #define FF_EXE_PATH_LEN MAXPATH
#elif defined(PATH_MAX)
    #define FF_EXE_PATH_LEN PATH_MAX
#else
    #define FF_EXE_PATH_LEN 260
#endif

static FFShellResult shellResult;

static void destroyShellResult(void* storage) {
    FFShellResult* result = storage;

    ffStrbufDestroy(&result->processName);
    ffStrbufDestroy(&result->exe);
    result->exeName = result->exe.chars; // `exeName` points into `exe`, which is empty again
    ffStrbufDestroy(&result->exePath);
    ffStrbufDestroy(&result->prettyName);
    ffStrbufDestroy(&result->version);
    result->pid = 0;
    result->ppid = 0;
    result->tty = -1;
}

static FFcacheEntry ffCacheEntryShell = {
    .name = "shell",
    .storage = &shellResult,
    .destroy = destroyShellResult,
};

const FFShellResult* ffDetectShell() {
    FFShellResult* result = &shellResult;

    if (!ffCacheBeginInit(&ffCacheEntryShell)) {
        return result;
    }

    ffStrbufInit(&result->processName);
    ffStrbufInitA(&result->exe, FF_EXE_PATH_LEN);
    result->exeName = result->exe.chars;
    ffStrbufInit(&result->exePath);
    ffStrbufInit(&result->version);
    result->pid = 0;
    result->ppid = 0;
    result->tty = -1;

    pid_t ppid = getppid();

    const char* ignoreParent = getenv("FFTS_IGNORE_PARENT");
    if (ignoreParent && ffStrEquals(ignoreParent, "1")) {
        FF_STRBUF_AUTO_DESTROY _ = ffStrbufCreate();
        ffProcessGetBasicInfoLinux(ppid, &_, &ppid, nullptr);
    }

    ppid = getShellInfo(result, ppid);

    /*
     * Emacs-terminal & shell detection block
     */
    bool detectedProcess = result->processName.length > 0;
    bool detectedEmacs =
        detectedProcess &&
        (isEmacsExecutable(result->exeName) ||
            isEmacsExecutable(result->processName.chars));

    const char* frontend = getEmacsTerminalNameHint();

    /*
     * eshell-detection-related code (& emacs-eat can also run eshell)
     * if shell is detected to be "emacs"
     */
    bool emacsEshell = detectedEmacs && isEmacsEshell();
    bool eatEshell =
        detectedEmacs &&
        frontend != nullptr &&
        strcmp(frontend, "eat") == 0;

    getUserShellFromEnv(result);

    if (result->processName.length > 0) {
        setShellInfoDetails(result);

        /*
         * eshell runs inside Emacs. Label the Emacs-hosted shell,
         * but only if we're really in Emacs.
         */
        if (emacsEshell || eatEshell) {
            ffStrbufSetS(&result->prettyName, result->exeName);
            ffStrbufAppendS(&result->prettyName, " (eshell)");
        }

        if (instance.config.general.detectVersion) {
            if (!fftsGetShellVersion(result->exePath.length > 0 ? &result->exePath : &result->exe, result->exeName, &result->version)) {
                ffStrbufClear(&result->version);
            }
        }
    }

    return result;
}

static FFTerminalResult terminalResult;

static void destroyTerminalResult(void* storage) {
    FFTerminalResult* result = storage;

    ffStrbufDestroy(&result->processName);
    ffStrbufDestroy(&result->exe);
    result->exeName = result->exe.chars; // `exeName` points into `exe`, which is empty again
    ffStrbufDestroy(&result->exePath);
    ffStrbufDestroy(&result->prettyName);
    ffStrbufDestroy(&result->version);
    ffStrbufDestroy(&result->tty);
    result->pid = 0;
    result->ppid = 0;
}

static FFcacheEntry ffCacheEntryTerminal = {
    .name = "terminal",
    .storage = &terminalResult,
    .destroy = destroyTerminalResult,
};

const FFTerminalResult* ffDetectTerminal() {
    FFTerminalResult* result = &terminalResult;

    if (!ffCacheBeginInit(&ffCacheEntryTerminal)) {
        return result;
    }

    ffStrbufInit(&result->processName);
    ffStrbufInitA(&result->exe, FF_EXE_PATH_LEN);
    result->exeName = result->exe.chars;
    ffStrbufInit(&result->exePath);
    ffStrbufInit(&result->version);
    ffStrbufInitS(&result->tty, ttyname(STDOUT_FILENO));
    result->pid = 0;
    result->ppid = 0;

    const char* frontend = getEmacsTerminalNameHint();
    const FFShellResult* shell = ffDetectShell();

    /*
     * Find Emacs in the process tree independently of INSIDE_EMACS.
     * That variable is inherited by descendants and does not prove
     * that Emacs is an ancestor of the current process.
     */
    pid_t startPid = shell->pid > 0
        ? (pid_t) shell->pid
        : (pid_t) shell->ppid;

    pid_t emacsPid = findEmacsAncestor(startPid);
    bool emacsTextMode =
        emacsPid > 1 && isEmacsTextMode(emacsPid);

    /*
     * First identify the terminal nearest to Fastfetch. This preserves
     * an external terminal launched as a child of graphical Emacs.
     */
    pid_t ppid = (pid_t) shell->ppid;
    if (ppid > 1)
        getTerminalInfo(result, ppid, emacsPid > 1);

    if (emacsTextMode) {
        /*
         * For text-mode Emacs, the containing terminal is above Emacs.
         * Retain a terminal found below Emacs; otherwise search upward
         * from Emacs to find its containing terminal.
         */
        /* bool terminalIsInsideEmacs = */
        /*     result->pid > 1 && */
        /*     isProcessAncestor(emacsPid, (pid_t) result->pid); */

      bool terminalContainsEmacs =
        result->pid > 1 &&
        isProcessAncestor((pid_t) result->pid, emacsPid);

        if (!terminalContainsEmacs) {
            ffStrbufClear(&result->processName);
            ffStrbufClear(&result->exe);
            result->exeName = result->exe.chars;
            ffStrbufClear(&result->exePath);
            result->pid = 0;
            result->ppid = 0;

            getTerminalInfo(result, emacsPid, emacsPid > 1);
        }
    }

    /* Environment-based detection is only a fallback. */
    getTerminalFromEnv(result);

    /*
     * Identify a terminal frontend hosted by graphical Emacs.
     * Require a verified Emacs ancestor; INSIDE_EMACS alone is not
     * sufficient, since external child terminals may inherit it.
     */
    bool graphicalEmacsTerminal =
        frontend != nullptr &&
        emacsPid > 1 &&
        !emacsTextMode &&
        shell->pid > 1 &&
        isProcessAncestor(emacsPid, (pid_t) shell->pid);

    if (graphicalEmacsTerminal) {
        ffStrbufSetS(&result->processName, "emacs");
        ffStrbufSetS(&result->exe, "emacs");
        result->exeName = result->exe.chars;
        ffStrbufClear(&result->exePath);
        result->pid = (uint32_t) emacsPid;
        result->ppid = 0;
    }

    if (result->processName.length > 0) {
        if (graphicalEmacsTerminal) {
            /*
             * The frontend is the terminal identity in graphical
             * Emacs. Do not let terminal-detail detection replace it.
             */
            ffStrbufSetS(&result->prettyName, "emacs");
            ffStrbufAppendS(&result->prettyName, " (");
            ffStrbufAppendS(&result->prettyName, frontend);
            ffStrbufAppendC(&result->prettyName, ')');
        } else {
            setTerminalInfoDetails(result);
        }

        /*
         * Only append the Emacs frontend to the version of the actual
         * containing terminal in text-mode Emacs.
         */
        bool terminalContainsEmacs =
            frontend != nullptr &&
            emacsTextMode &&
            emacsPid > 1 &&
            result->pid > 1 &&
            isProcessAncestor((pid_t) result->pid, emacsPid);

        bool environmentFallback =
            frontend != nullptr &&
            emacsTextMode &&
            emacsPid > 1 &&
            result->pid == 0;

        if (instance.config.general.detectVersion) {
            if (!fftsGetTerminalVersion(&result->processName, result->exePath.length > 0 ? &result->exePath : &result->exe, &result->version)) {
                ffStrbufClear(&result->version);
            }
        }

        /*
         * Preserve the outer terminal's prettyName. Keep the Emacs
         * frontend annotation in the version field for text-mode Emacs.
         */
        if (terminalContainsEmacs || environmentFallback) {
            if (result->version.length > 0)
                ffStrbufAppendC(&result->version, ' ');

            ffStrbufAppendS(&result->version, "[emacs: ");
            ffStrbufAppendS(&result->version, frontend);
            ffStrbufAppendC(&result->version, ']');
        }
    }

    return result;
}
