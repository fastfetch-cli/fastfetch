#include "fastfetch.h"
#include "common/processing.h"
#include "common/io/io.h"
#include "common/time.h"
#include "util/stringUtils.h"
#include "util/mallocHelper.h"

#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <poll.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/wait.h>

#if defined(__FreeBSD__) || defined(__APPLE__)
    #include <sys/types.h>
    #include <sys/user.h>
    #include <sys/sysctl.h>
#endif
#if defined(__APPLE__)
    #include <libproc.h>
#elif defined(__sun)
    #include <procfs.h>
#endif

enum { FF_PIPE_BUFSIZ = 8192 };

static inline int ffPipe2(int *fds, int flags)
{
    #ifdef __APPLE__
        if(pipe(fds) == -1)
            return -1;
        fcntl(fds[0], F_SETFL, fcntl(fds[0], F_GETFL) | flags);
        fcntl(fds[1], F_SETFL, fcntl(fds[1], F_GETFL) | flags);
        return 0;
    #else
        return pipe2(fds, flags);
    #endif
}

const char* ffProcessAppendOutput(FFstrbuf* buffer, char* const argv[], bool useStdErr)
{
    int pipes[2];
    const int timeout = instance.config.general.processingTimeout;

    if(ffPipe2(pipes, O_CLOEXEC) == -1)
        return "pipe() failed";

    pid_t childPid = fork();
    if(childPid == -1)
    {
        close(pipes[0]);
        close(pipes[1]);
        return "fork() failed";
    }

    //Child
    if(childPid == 0)
    {
        int nullFile = open("/dev/null", O_WRONLY | O_CLOEXEC);
        dup2(pipes[1], useStdErr ? STDERR_FILENO : STDOUT_FILENO);
        dup2(nullFile, useStdErr ? STDOUT_FILENO : STDERR_FILENO);
        setenv("LANG", "C", 1);
        execvp(argv[0], argv);
        _exit(127);
    }

    //Parent
    close(pipes[1]);

    int FF_AUTO_CLOSE_FD childPipeFd = pipes[0];
    char str[FF_PIPE_BUFSIZ];

    while(1)
    {
        if (timeout >= 0)
        {
            struct pollfd pollfd = { childPipeFd, POLLIN, 0 };
            if (poll(&pollfd, 1, timeout) == 0)
            {
                kill(childPid, SIGTERM);
                waitpid(childPid, NULL, 0);
                return "poll(&pollfd, 1, timeout) timeout (try increasing --processing-timeout)";
            }
            else if (pollfd.revents & POLLERR)
            {
                kill(childPid, SIGTERM);
                waitpid(childPid, NULL, 0);
                return "poll(&pollfd, 1, timeout) error";
            }
        }

        ssize_t nRead = read(childPipeFd, str, FF_PIPE_BUFSIZ);
        if (nRead > 0)
            ffStrbufAppendNS(buffer, (uint32_t) nRead, str);
        else if (nRead == 0)
        {
            int stat_loc = 0;
            if (waitpid(childPid, &stat_loc, 0) == childPid)
            {
                if (!WIFEXITED(stat_loc))
                    return "child process exited abnormally";
                if (WEXITSTATUS(stat_loc) == 127)
                    return "command was not found";
                // We only handle 127 as an error. See `getTerminalVersionUrxvt` in `terminalshell.c`
                return NULL;
            }
            return "waitpid() failed";
        }
        else if (nRead < 0)
            break;
    };

    return "read(childPipeFd, str, FF_PIPE_BUFSIZ) failed";
}

void ffProcessGetInfoLinux(pid_t pid, FFstrbuf* processName, FFstrbuf* exe, const char** exeName, FFstrbuf* exePath)
{
    assert(processName->length > 0);
    ffStrbufClear(exe);

    #ifdef __linux__

    char filePath[64];
    snprintf(filePath, sizeof(filePath), "/proc/%d/cmdline", (int)pid);

    if(ffReadFileBuffer(filePath, exe))
    {
        ffStrbufRecalculateLength(exe); //Trim the arguments
        ffStrbufTrimRightSpace(exe);
        ffStrbufTrimLeft(exe, '-'); //Login shells start with a dash
    }

    if (exePath)
    {
        snprintf(filePath, sizeof(filePath), "/proc/%d/exe", (int)pid);
        ffStrbufEnsureFixedLengthFree(exePath, PATH_MAX);
        ssize_t length = readlink(filePath, exePath->chars, exePath->allocated - 1);
        if (length > 0) // doesn't contain trailing NUL
        {
            exePath->chars[length] = '\0';
            exePath->length = (uint32_t) length;
        }
    }

    #elif defined(__APPLE__)

    size_t len = 0;
    int mibs[] = { CTL_KERN, KERN_PROCARGS2, pid };
    if (sysctl(mibs, sizeof(mibs) / sizeof(*mibs), NULL, &len, NULL, 0) == 0)
    {// try get arg0
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
            if (exePath)
                ffStrbufSetNS(exePath, (uint32_t) (arg0 - realExePath), realExePath);

            do arg0++; while (*arg0 == '\0');
            assert(arg0 < procArgs2 + len);

            if (argc > 1)
            {
                // #977
                const char* p = strrchr(arg0, '/');
                if (p)
                    p++;
                else
                    p = arg0;
                if (ffStrStartsWithIgnCase(p, "python")) // /opt/homebrew/Cellar/python@3.12/3.12.3/Frameworks/Python.framework/Versions/3.12/Resources/Python.app/Contents/MacOS/Python /Users/carter/.local/bin/xonsh
                    arg0 = p + strlen(p) + 1;
            }

            if (*arg0 == '-') arg0++; // Login shells

            ffStrbufSetS(exe, arg0);
        }
    }
    else
    {
        ffStrbufEnsureFixedLengthFree(exe, PATH_MAX);
        int length = proc_pidpath(pid, exe->chars, exe->allocated);
        if (length > 0)
        {
            exe->length = (uint32_t) length;
            if (exePath)
                ffStrbufSet(exePath, exe);
        }
    }

    #elif defined(__FreeBSD__)

    size_t size = ARG_MAX;
    FF_AUTO_FREE char* args = malloc(size);

    static_assert(ARG_MAX > PATH_MAX, "");

    if(exePath && sysctl(
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
                p++;
            else
                p = arg0;
            if (ffStrStartsWith(p, "python")) // /usr/local/bin/python3.9 /home/carter/.local/bin/xonsh
            {
                arg0 += arg0Len + 1;
            }
        }
        if (arg0[0] == '-') arg0++;
        ffStrbufSetS(exe, arg0);
    }

    #elif defined(__sun)

    char filePath[PATH_MAX];
    snprintf(filePath, sizeof(filePath), "/proc/%d/psinfo", (int) pid);
    psinfo_t proc;
    if (ffReadFileData(filePath, sizeof(proc), &proc) == sizeof(proc))
    {
        ffStrbufSetS(exe, proc.pr_psargs);
        ffStrbufSubstrBeforeFirstC(exe, ' ');
    }

    if (exePath)
    {
        snprintf(filePath, sizeof(filePath), "/proc/%d/path/a.out", (int) pid);
        ffStrbufEnsureFixedLengthFree(exePath, PATH_MAX);
        ssize_t length = readlink(filePath, exePath->chars, exePath->allocated - 1);
        if (length > 0) // doesn't contain trailing NUL
        {
            exePath->chars[length] = '\0';
            exePath->length = (uint32_t) length;
        }
    }

    #endif

    if(exe->length == 0)
        ffStrbufSet(exe, processName);

    assert(exe->length > 0);
    uint32_t lastSlashIndex = ffStrbufLastIndexC(exe, '/');
    if(lastSlashIndex < exe->length)
        *exeName = exe->chars + lastSlashIndex + 1;
}

const char* ffProcessGetBasicInfoLinux(pid_t pid, FFstrbuf* name, pid_t* ppid, int32_t* tty)
{
    if (pid <= 0)
        return "Invalid pid";

    #ifdef __linux__

    char procFilePath[64];
    if (ppid)
    {
        snprintf(procFilePath, sizeof(procFilePath), "/proc/%d/stat", (int)pid);
        char buf[PROC_FILE_BUFFSIZ];
        ssize_t nRead = ffReadFileData(procFilePath, sizeof(buf) - 1, buf);
        if(nRead <= 8)
            return "ffReadFileData(/proc/pid/stat, PROC_FILE_BUFFSIZ-1, buf) failed";
        buf[nRead] = '\0';

        *ppid = 0;
        static_assert(sizeof(*ppid) == sizeof(int), "");

        ffStrbufEnsureFixedLengthFree(name, 255);
        int tty_;
        if(
            sscanf(buf, "%*s (%255[^)]) %*c %d %*d %*d %d", name->chars, ppid, &tty_) < 2 || //stat (comm) state ppid pgrp session tty
            name->chars[0] == '\0'
        )
            return "sscanf(stat) failed";

        ffStrbufRecalculateLength(name);
        if (tty)
            *tty = tty_ & 0xFF;
    }
    else
    {
        snprintf(procFilePath, sizeof(procFilePath), "/proc/%d/comm", (int)pid);
        ssize_t nRead = ffReadFileBuffer(procFilePath, name);
        if(nRead <= 0)
            return "ffReadFileBuffer(/proc/pid/comm, name) failed";
        ffStrbufTrimRightSpace(name);
    }

    #elif defined(__APPLE__)

    struct kinfo_proc proc;
    size_t size = sizeof(proc);
    if(sysctl(
        (int[]){CTL_KERN, KERN_PROC, KERN_PROC_PID, pid}, 4,
        &proc, &size,
        NULL, 0
    ))
        return "sysctl(KERN_PROC_PID) failed";

    ffStrbufSetS(name, proc.kp_proc.p_comm); //trancated to 16 chars
    if (ppid)
        *ppid = (pid_t)proc.kp_eproc.e_ppid;
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

    ffStrbufSetS(name, proc.ki_comm);
    if (ppid)
        *ppid = (pid_t)proc.ki_ppid;
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

    #elif defined(__sun)
    char path[128];
    snprintf(path, sizeof(path), "/proc/%d/psinfo", (int) pid);
    psinfo_t proc;
    if (ffReadFileData(path, sizeof(proc), &proc) != sizeof(proc))
        return "ffReadFileData(psinfo) failed";

    ffStrbufSetS(name, proc.pr_fname);
    if (ppid)
        *ppid = proc.pr_ppid;
    if (tty)
        *tty = (int) proc.pr_ttydev;

    #else

    return "Unsupported platform";

    #endif

    return NULL;
}
