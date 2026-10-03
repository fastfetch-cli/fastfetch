#include "fastfetch.h"
#include "common/debug.h"
#include "common/processing.h"
#include "common/io.h"
#include "common/strutil.h"
#include "common/mallocHelper.h"
#include "common/time.h"

#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <poll.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/wait.h>

#if !(__ANDROID__ || __OpenBSD__)
    #include <spawn.h>
#endif

#if defined(__FreeBSD__) || defined(__APPLE__)
    #include <sys/types.h>
    #include <sys/user.h>
    #include <sys/sysctl.h>
#endif
#if defined(__APPLE__)
    #include <libproc.h>
#elif defined(__sun)
    #include <procfs.h>
#elif defined(__OpenBSD__)
    #include <sys/param.h>
    #include <sys/sysctl.h>
#elif defined(__NetBSD__)
    #include <sys/types.h>
    #include <sys/sysctl.h>
#elif defined(__HAIKU__)
    #include <OS.h>
    #include <image.h>
#endif

#ifndef environ
extern char** environ;
#endif

enum { FF_PIPE_BUFSIZ = 8192 };

static inline int ffPipe2(int* readEnd, int* writeEnd, int flags) {
    int fds[2];
#ifndef FF_HAVE_PIPE2
    if (pipe(fds) == -1) {
        return -1;
    }
    fcntl(fds[0], F_SETFL, fcntl(fds[0], F_GETFL) | flags);
    fcntl(fds[1], F_SETFL, fcntl(fds[1], F_GETFL) | flags);
#else
    if (pipe2(fds, flags) == -1) {
        return -1;
    }
#endif
    *readEnd = fds[0];
    *writeEnd = fds[1];
    return 0;
}

// Not thread-safe
const char* ffProcessSpawn(char* const argv[], bool useStdErr, FFNativeFD stdinFd, FFProcessHandle* outHandle) {
    FF_AUTO_CLOSE_FD int pipeRead = -1;
    FF_AUTO_CLOSE_FD int pipeWrite = -1;
    if (ffPipe2(&pipeRead, &pipeWrite, O_CLOEXEC) == -1) {
        FF_DEBUG("pipe() failed: %s", strerror(errno));
        return "pipe() failed";
    }

    pid_t childPid = -1;
    int nullFile = ffGetNullFD();

#if !(__ANDROID__ || __OpenBSD__)

    // NetBSD / Darwin: native syscall
    // Linux (glibc): clone3-execve
    // FreeBSD: vfork-execve
    // illumos: vforkx-execve
    // OpenBSD / Android (bionic): fork-execve

    posix_spawn_file_actions_t file_actions;
    posix_spawn_file_actions_init(&file_actions);
    posix_spawn_file_actions_adddup2(&file_actions, pipeWrite, useStdErr ? STDERR_FILENO : STDOUT_FILENO);
    posix_spawn_file_actions_adddup2(&file_actions, nullFile, useStdErr ? STDOUT_FILENO : STDERR_FILENO);
    if (ffIsValidNativeFD(stdinFd)) {
        posix_spawn_file_actions_adddup2(&file_actions, stdinFd, STDIN_FILENO);
    }

    static char* oldLang = nullptr;
    static int langIndex = -1;

    if (langIndex >= 0) {
        // Found before
        if (oldLang) // oldLang was set only if it needed to be changed
        {
            if (environ[langIndex] != oldLang) {
                // environ is changed outside of this function
                langIndex = -1;
            } else {
                environ[langIndex] = (char*) "LANG=C.UTF-8";
            }
        }
    }
    if (langIndex < 0) {
        for (int i = 0; environ[i] != nullptr; i++) {
            if (ffStrStartsWith(environ[i], "LANG=")) {
                langIndex = i;
                const char* langValue = environ[i] + 5; // Skip "LANG="
                if (ffStrEqualsIgnCase(langValue, "C") ||
                    ffStrStartsWithIgnCase(environ[i], "C.") ||
                    ffStrEqualsIgnCase(langValue, "en_US") ||
                    ffStrStartsWithIgnCase(langValue, "en_US.")) {
                    break; // No need to change LANG
                }
                oldLang = environ[i];
                environ[i] = (char*) "LANG=C.UTF-8"; // Set LANG to C.UTF-8 for consistent output
                break;
            }
        }
    }

    int ret = posix_spawnp(&childPid, argv[0], &file_actions, nullptr, argv, environ);

    if (oldLang) {
        environ[langIndex] = oldLang;
    }

    posix_spawn_file_actions_destroy(&file_actions);

    if (ret != 0) {
        FF_DEBUG("posix_spawnp() failed: %s", strerror(ret));
        if (ret == ENOENT) {
            return "command not found";
        }
        return "posix_spawnp() failed";
    }

#else

    // https://github.com/termux/termux-packages/issues/25369
    childPid = fork();
    if (childPid == -1) {
        FF_DEBUG("fork() failed: %s", strerror(errno));
        return "fork() failed";
    }

    if (childPid == 0) {
        // Child process
        dup2(pipeWrite, useStdErr ? STDERR_FILENO : STDOUT_FILENO);
        dup2(nullFile, useStdErr ? STDOUT_FILENO : STDERR_FILENO);
        if (ffIsValidNativeFD(stdinFd)) {
            dup2(stdinFd, STDIN_FILENO);
        }
        putenv("LANG=C.UTF-8");
        execvp(argv[0], argv);
        _exit(127);
    }

#endif

    outHandle->pid = childPid;
    outHandle->pipeRead = pipeRead;
    pipeRead = -1; // ownership moved into outHandle, which ffProcessReadOutput() lets go
    return nullptr;
}

static const char* ffProcessCheckExitStatus(int stat_loc) {
    if (!WIFEXITED(stat_loc)) {
        return "child process exited abnormally";
    }
    if (WEXITSTATUS(stat_loc) == 127) {
        FF_DEBUG("command not found");
        return "command not found";
    }
    // We only handle 127 as an error. See `getTerminalVersionUrxvt` in `terminalshell.c`
    return nullptr;
}

const char* ffProcessReadOutput(FFProcessHandle* handle, FFstrbuf* buffer) {
    assert(handle->pipeRead != -1);
    assert(handle->pid != -1);

    const int32_t timeout = instance.config.general.processingTimeout;
    FF_AUTO_CLOSE_FD int childPipeFd = handle->pipeRead;
    pid_t childPid = handle->pid;
    handle->pipeRead = -1;
    handle->pid = -1;
    char str[FF_PIPE_BUFSIZ];
    double lastOutputTick = ffTimeGetTick();
    bool childExited = false;
    int stat_loc = 0;
    enum { FF_PROCESS_POLL_INTERVAL = 50 };

    for (;;) {
        // poll() has no portable way to watch a child process on every supported POSIX system.
        // Use short slices so a child that exits while a daemonized descendant still holds the
        // output pipe open can be observed without process-global signal handling.
        int pollTimeout = FF_PROCESS_POLL_INTERVAL;
        if (childExited) {
            pollTimeout = 0;
        } else if (timeout >= 0) {
            double remaining = (double) timeout - (ffTimeGetTick() - lastOutputTick);
            if (remaining <= 0) {
                pollTimeout = 0;
            } else if (remaining < FF_PROCESS_POLL_INTERVAL) {
                pollTimeout = (int) remaining;
            }
        }

        struct pollfd pollfd = { childPipeFd, POLLIN, 0 };
        int pollret = poll(&pollfd, 1, pollTimeout);
        bool pollInterrupted = pollret < 0 && errno == EINTR;
        if (pollret < 0 && !pollInterrupted) {
            kill(childPid, SIGTERM);
            while (waitpid(childPid, nullptr, 0) < 0 && errno == EINTR) {}
            return "poll(&pollfd, 1, timeout) error: pollret < 0";
        }
        if (pollret > 0 && (pollfd.revents & (POLLERR | POLLNVAL))) {
            kill(childPid, SIGTERM);
            while (waitpid(childPid, nullptr, 0) < 0 && errno == EINTR) {}
            return "poll(&pollfd, 1, timeout) error: pollfd.revents & POLLERR";
        }

        if (!childExited) {
            pid_t waitResult;
            do {
                waitResult = waitpid(childPid, &stat_loc, WNOHANG);
            } while (waitResult < 0 && errno == EINTR);
            if (waitResult == childPid || (waitResult < 0 && errno == ECHILD)) {
                childExited = true;
            } else if (waitResult < 0) {
                return "waitpid(childPid, &stat_loc, WNOHANG) failed";
            }
        }

        if (pollInterrupted) {
            continue;
        }

        if (pollret > 0 && (pollfd.revents & (POLLIN | POLLHUP))) {
            ssize_t nRead = read(childPipeFd, str, FF_PIPE_BUFSIZ);
            if (nRead > 0) {
                ffStrbufAppendNS(buffer, (uint32_t) nRead, str);
                lastOutputTick = ffTimeGetTick();
                continue;
            } else if (nRead == 0) {
                if (!childExited && childPid > 0) {
                    pid_t waitResult;
                    do {
                        waitResult = waitpid(childPid, &stat_loc, 0);
                    } while (waitResult < 0 && errno == EINTR);
                    childExited = waitResult == childPid;
                }
                return childExited ? ffProcessCheckExitStatus(stat_loc) : nullptr;
            } else if (errno != EINTR && errno != EAGAIN && errno != EWOULDBLOCK) {
                FF_DEBUG("read(childPipeFd, str, FF_PIPE_BUFSIZ) failed: %s", strerror(errno));
                return "read(childPipeFd, str, FF_PIPE_BUFSIZ) failed";
            }
        }

        if (childExited) {
            return ffProcessCheckExitStatus(stat_loc);
        }

        if (timeout >= 0 && ffTimeGetTick() - lastOutputTick >= (double) timeout) {
            FF_DEBUG("poll(&pollfd, 1, timeout) timeout (try increasing --processing-timeout)");
            kill(childPid, SIGTERM);
            while (waitpid(childPid, nullptr, 0) < 0 && errno == EINTR) {}
            return "poll(&pollfd, 1, timeout) timeout (try increasing --processing-timeout)";
        }
    }
}

void ffProcessGetInfoLinux(pid_t pid, FFstrbuf* processName, FFstrbuf* exe, const char** exeName, FFstrbuf* exePath) {
    assert(processName->length > 0);
    ffStrbufClear(exe);
    if (exePath) {
        ffStrbufClear(exePath);
    }

#if defined(__linux__) || defined(__GNU__)

    char filePath[64];
    snprintf(filePath, sizeof(filePath), "/proc/%d/cmdline", (int) pid);

    if (ffReadFileBuffer(filePath, exe)) {
        const char* p = exe->chars;
        uint32_t len = (uint32_t) strlen(p);

        if (len + 1 < exe->length) {
            const char* name = memrchr(p, '/', len);
            if (name) {
                name++;
            } else {
                name = p;
            }

            // For interpreters, try to find the real script path in the arguments
            if (ffStrStartsWith(name, "python")
    #ifndef __ANDROID__
                || ffStrEquals(name, "guile") // for shepherd
    #endif
            ) {
                // `cmdline` always ends with a trailing '\0', and ffReadFileBuffer appends another \0
                // So `exe->chars` is always double '\0' terminated
                for (p = p + len + 1; *p && *p == '-'; p += strlen(p) + 1) { // Skip arguments
                    assert(p - exe->chars < exe->allocated);
                }
                if (*p) {
                    len = (uint32_t) strlen(p);
                    memmove(exe->chars, p, len + 1);
                }
            }
        }

        assert(len < exe->allocated);
        exe->length = len;
        ffStrbufTrimLeft(exe, '-'); // Login shells start with a dash
    }

    if (exePath) {
        snprintf(filePath, sizeof(filePath), "/proc/%d/exe", (int) pid);
        char buf[PATH_MAX];
        ssize_t length = readlink(filePath, buf, PATH_MAX - 1);
        if (length > 0) // doesn't contain trailing NUL
        {
            buf[length] = '\0';
            // When the process is a deleted executable, the resolved path is like `/usr/bin/app (deleted)`
            // But we can still access the binary via `/proc/pid/exe`. See #2136
            if (ffPathExists(buf, FF_PATHTYPE_ANY)) {
                ffStrbufSetNS(exePath, (uint32_t) length, buf);
            }
        }

        if (exePath->length == 0 && access(filePath, X_OK) == 0) {
            ffStrbufSetS(exePath, filePath);
        }
    }

#elif defined(__APPLE__)

    size_t len = 0;
    int mibs[] = { CTL_KERN, KERN_PROCARGS2, pid };
    if (sysctl(mibs, ARRAY_SIZE(mibs), nullptr, &len, nullptr, 0) == 0) { // try get arg0
        // don't know why if don't let len longer, proArgs2 and len will change during the following sysctl() in old MacOS version.
        len++;
        FF_AUTO_FREE char* const procArgs2 = malloc(len);
        if (sysctl(mibs, ARRAY_SIZE(mibs), procArgs2, &len, nullptr, 0) == 0) {
            // https://gist.github.com/nonowarn/770696#file-getargv-c-L46
            uint32_t argc = *(uint32_t*) procArgs2;
            const char* realExePath = procArgs2 + sizeof(argc);

            const char* arg0 = memchr(realExePath, '\0', len - (size_t) (realExePath - procArgs2));
            if (exePath) {
                ffStrbufSetNS(exePath, (uint32_t) (arg0 - realExePath), realExePath);
            }

            do {
                arg0++;
            } while (*arg0 == '\0');
            assert(arg0 < procArgs2 + len);

            if (argc > 1) {
                // #977
                const char* p = strrchr(arg0, '/');
                if (p) {
                    p++;
                } else {
                    p = arg0;
                }
                if (ffStrStartsWithIgnCase(p, "python")) { // /opt/homebrew/Cellar/python@3.12/3.12.3/Frameworks/Python.framework/Versions/3.12/Resources/Python.app/Contents/MacOS/Python /Users/carter/.local/bin/xonsh
                    arg0 = p + strlen(p) + 1;
                }
            }

            if (*arg0 == '-') {
                arg0++; // Login shells
            }

            ffStrbufSetS(exe, arg0);
        }
    }

    if (exePath || exe->length == 0) {
        char buf[PROC_PIDPATHINFO_MAXSIZE];
        int length = proc_pidpath(pid, buf, ARRAY_SIZE(buf));
        if (length > 0) {
            if (exe->length == 0) {
                ffStrbufSetNS(exe, (uint32_t) length, buf);
            }
            if (exePath) {
                // We don't use exec_path above as exePath because it's a relative path and can be different
                // from the actual executable being run (for example, when the original file is moved)
                ffStrbufSetNS(exePath, (uint32_t) length, buf);
            }
        }
    }

#elif defined(__FreeBSD__) || defined(__NetBSD__)

    size_t size = ARG_MAX;
    FF_AUTO_FREE char* args = malloc(size);

    static_assert(ARG_MAX > PATH_MAX, "");

    if (exePath && sysctl((int[]) { CTL_KERN,
    #if __FreeBSD__
                              KERN_PROC,
                              KERN_PROC_PATHNAME,
                              pid
    #else
                              KERN_PROC_ARGS,
                              pid,
                              KERN_PROC_PATHNAME
    #endif
                          },
                       4,
                       args,
                       &size,
                       nullptr,
                       0) == 0)
        ffStrbufSetNS(exePath, (uint32_t) (size - 1), args);

    size = ARG_MAX;
    if (sysctl(
            (int[]) { CTL_KERN,
    #if __FreeBSD__
                KERN_PROC,
                KERN_PROC_ARGS,
                pid
    #else
                KERN_PROC_ARGS,
                pid,
                KERN_PROC_ARGV,
    #endif
            },
            4,
            args,
            &size,
            nullptr,
            0) == 0) {
        char* arg0 = args;
        size_t arg0Len = strlen(args);
        if (size > arg0Len + 1) {
            char* p = (char*) memrchr(args, '/', arg0Len);
            if (p) {
                p++;
            } else {
                p = arg0;
            }
            if (ffStrStartsWith(p, "python")) // /usr/local/bin/python3.9 /home/carter/.local/bin/xonsh
            {
                arg0 += arg0Len + 1;
            }
        }
        if (arg0[0] == '-') {
            arg0++;
        }
        ffStrbufSetS(exe, arg0);
    }

#elif defined(__sun)

    char filePath[128];
    snprintf(filePath, sizeof(filePath), "/proc/%d/psinfo", (int) pid);
    psinfo_t proc;
    if (ffReadFileData(filePath, sizeof(proc), &proc) == sizeof(proc)) {
        const char* args = proc.pr_psargs;
        if (args[0] == '-') {
            ++args;
        }
        const char* end = strchr(args, ' ');
        ffStrbufSetNS(exe, end ? (uint32_t) (end - args) : (uint32_t) strlen(args), args);
    }

    if (exePath) {
        snprintf(filePath, sizeof(filePath), "/proc/%d/path/a.out", (int) pid);
        char buf[PATH_MAX];
        ssize_t length = readlink(filePath, buf, PATH_MAX - 1);
        if (length > 0) // doesn't contain trailing NUL
        {
            buf[length] = '\0';
            ffStrbufSetNS(exePath, (uint32_t) length, buf);
        }
    }

#elif defined(__OpenBSD__)

    char argvBuf[ARG_MAX];
    size_t argvSize = sizeof(argvBuf);
    int argvMib[] = { CTL_KERN, KERN_PROC_ARGS, pid, KERN_PROC_ARGV };
    if (sysctl(argvMib, ARRAY_SIZE(argvMib), argvBuf, &argvSize, nullptr, 0) == 0) {
        // The buffer is filled with an array of char pointers followed by the strings themselves
        char** argv = (char**) argvBuf;
        if (argv[0] && (char*) argv[0] >= argvBuf && (char*) argv[0] < argvBuf + argvSize) {
            const char* arg0 = argv[0];
            if (arg0[0] == '-') {
                arg0++;
            }
            ffStrbufSetS(exe, arg0);
        }
    }

#elif defined(__HAIKU__)

    image_info info;
    int32 cookie = 0;

    while (get_next_image_info(pid, &cookie, &info) == B_OK) {
        if (info.type != B_APP_IMAGE) {
            continue;
        }
        ffStrbufSetS(exe, info.name);

        if (exePath) {
            ffStrbufSet(exePath, exe);
        }
        break;
    }

#endif

    if (exe->length == 0) {
        ffStrbufSet(exe, processName);
    }

    assert(exe->length > 0);
    uint32_t lastSlashIndex = ffStrbufLastIndexC(exe, '/');
    if (lastSlashIndex < exe->length) {
        *exeName = exe->chars + lastSlashIndex + 1;
    }
}

const char* ffProcessGetBasicInfoLinux(pid_t pid, FFstrbuf* name, pid_t* ppid, int32_t* tty) {
    if (pid <= 0) {
        FF_DEBUG("Invalid pid");
        return "Invalid pid";
    }

#if defined(__linux__) || defined(__GNU__)

    char procFilePath[64];
    #if __linux__
    if (ppid || tty)
    #endif
    {
        snprintf(procFilePath, sizeof(procFilePath), "/proc/%d/stat", (int) pid);
        char buf[PROC_FILE_BUFFSIZ];
        ssize_t nRead = ffReadFileData(procFilePath, sizeof(buf) - 1, buf);
        if (nRead <= 8) {
            FF_DEBUG("ffReadFileData(/proc/pid/stat, PROC_FILE_BUFFSIZ-1, buf) failed");
            return "ffReadFileData(/proc/pid/stat, PROC_FILE_BUFFSIZ-1, buf) failed";
        }
        buf[nRead] = '\0'; // pid (comm) state ppid pgrp session tty

        const char* pState = nullptr;

        {
            // comm in `/proc/pid/stat` is not encoded, and may contain ' ', ')' or even `\n`
            const char* start = memchr(buf, '(', (size_t) nRead);
            if (!start) {
                FF_DEBUG("memchr(stat, '(') failed");
                return "memchr(stat, '(') failed";
            }
            start++;
            const char* end = memrchr(start, ')', (size_t) nRead - (size_t) (start - buf));
            if (!end) {
                FF_DEBUG("memrchr(stat, ')') failed");
                return "memrchr(stat, ')') failed";
            }
            ffStrbufSetNS(name, (uint32_t) (end - start), start);
            ffStrbufTrimRightSpace(name);
            if (name->chars[0] == '\0') {
                FF_DEBUG("process name is empty");
                return "process name is empty";
            }
            pState = end + 2; // skip ") "
        }

    #if !__linux__
        if (ppid || tty)
    #endif
        {
            int ppid_, tty_;
            if (sscanf(pState + 2, "%d %*d %*d %d", &ppid_, &tty_) < 2) {
                FF_DEBUG("sscanf(stat) failed");
                return "sscanf(stat) failed";
            }

            if (ppid) {
                *ppid = (pid_t) ppid_;
            }
            if (tty) {
                *tty = tty_ & 0xFF;
            }
        }
    }
    #if __linux__
    else {
        snprintf(procFilePath, sizeof(procFilePath), "/proc/%d/comm", (int) pid);
        ssize_t nRead = ffReadFileBuffer(procFilePath, name);
        if (nRead <= 0) {
            FF_DEBUG("ffReadFileBuffer(/proc/pid/comm, name) failed");
            return "ffReadFileBuffer(/proc/pid/comm, name) failed";
        }
        ffStrbufTrimRightSpace(name);
    }
    #endif

#elif defined(__APPLE__)

    struct kinfo_proc proc;
    size_t size = sizeof(proc);
    if (sysctl(
            (int[]) { CTL_KERN, KERN_PROC, KERN_PROC_PID, pid }, 4, &proc, &size, nullptr, 0)) {
        FF_DEBUG("sysctl(KERN_PROC_PID) failed: %s", strerror(errno));
        return "sysctl(KERN_PROC_PID) failed";
    }

    ffStrbufSetS(name, proc.kp_proc.p_comm); // truncated to 16 chars
    if (ppid) {
        *ppid = (pid_t) proc.kp_eproc.e_ppid;
    }
    if (tty) {
        *tty = ((proc.kp_eproc.e_tdev >> 24) & 0xFF) == 0x10
            ? proc.kp_eproc.e_tdev & 0xFFFFFF
            : -1;
    }

#elif defined(__FreeBSD__)

    #ifdef __DragonFly__
        #define ki_comm kp_comm
        #define ki_ppid kp_ppid
        #define ki_tdev kp_tdev
        #define ki_flag kp_flags
    #endif

    struct kinfo_proc proc;
    size_t size = sizeof(proc);
    if (sysctl(
            (int[]) { CTL_KERN, KERN_PROC, KERN_PROC_PID, pid }, 4, &proc, &size, nullptr, 0)) {
        FF_DEBUG("sysctl(KERN_PROC_PID) failed: %s", strerror(errno));
        return "sysctl(KERN_PROC_PID) failed";
    }

    ffStrbufSetS(name, proc.ki_comm);
    if (ppid) {
        *ppid = (pid_t) proc.ki_ppid;
    }
    if (tty) {
        if (proc.ki_tdev != NODEV && proc.ki_flag & P_CONTROLT) {
            const char* ttyName = devname(proc.ki_tdev, S_IFCHR);
            if (ffStrStartsWith(ttyName, "pts/")) {
                *tty = (int32_t) strtol(ttyName + strlen("pts/"), nullptr, 10);
            } else {
                *tty = -1;
            }
        } else {
            *tty = -1;
        }
    }

#elif defined(__NetBSD__)

    struct kinfo_proc2 proc;
    size_t size = sizeof(proc);
    if (sysctl(
            (int[]) { CTL_KERN, KERN_PROC2, KERN_PROC_PID, pid, sizeof(proc), 1 }, 6, &proc, &size, nullptr, 0) != 0) {
        FF_DEBUG("sysctl(KERN_PROC_PID) failed: %s", strerror(errno));
        return "sysctl(KERN_PROC_PID) failed";
    }

    ffStrbufSetS(name, proc.p_comm);
    if (ppid) {
        *ppid = (pid_t) proc.p_ppid;
    }
    if (tty) {
        if (proc.p_flag & P_CONTROLT) {
            const char* ttyName = devname(proc.p_tdev, S_IFCHR);
            if (ffStrStartsWith(ttyName, "pts/")) {
                *tty = (int32_t) strtol(ttyName + strlen("pts/"), nullptr, 10);
            } else {
                *tty = -1;
            }
        } else {
            *tty = -1;
        }
    }

#elif defined(__sun)
    char path[128];
    snprintf(path, sizeof(path), "/proc/%d/psinfo", (int) pid);
    psinfo_t proc;
    if (ffReadFileData(path, sizeof(proc), &proc) != sizeof(proc)) {
        FF_DEBUG("ffReadFileData(psinfo) failed");
        return "ffReadFileData(psinfo) failed";
    }

    ffStrbufSetS(name, proc.pr_fname);
    if (ppid) {
        *ppid = proc.pr_ppid;
    }
    if (tty) {
        *tty = (int) proc.pr_ttydev;
    }

#elif defined(__OpenBSD__)

    struct kinfo_proc proc;
    size_t size = sizeof(proc);
    int mib[] = { CTL_KERN, KERN_PROC, KERN_PROC_PID, pid, (int) sizeof(struct kinfo_proc), 1 };
    if (sysctl(mib, ARRAY_SIZE(mib), &proc, &size, nullptr, 0) == 0) {
        ffStrbufSetS(name, proc.p_comm);
        if (ppid) {
            *ppid = proc.p_ppid;
        }
        if (tty) {
            *tty = (int) proc.p_tdev;
        }
    } else {
        FF_DEBUG("sysctl(KERN_PROC_PID) failed: %s", strerror(errno));
        return "sysctl(KERN_PROC_PID) failed";
    }

#elif defined(__HAIKU__)

    team_info info;
    if (get_team_info(pid, &info) == B_OK) {
        ffStrbufSetS(name, info.name);
        if (ppid) {
            *ppid = info.parent;
        }
    }

    FF_UNUSED(tty);

#else

    FF_DEBUG("Unsupported platform");
    return "Unsupported platform";

#endif

    return nullptr;
}
