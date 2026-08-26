#include "lm.h"
#include "common/io.h"
#include "common/mallocHelper.h"
#include "common/properties.h"
#include "common/processing.h"
#include "common/strutil.h"

#ifdef FF_HAVE_DBUS
    #include "common/dbus.h"
#endif

#include <unistd.h>

#if __FreeBSD__
    #include <sys/sysctl.h>
    #include <sys/types.h>
    #include <sys/user.h>
#elif __OpenBSD__
    #include <sys/param.h>
    #include <sys/sysctl.h>
    #include <kvm.h>
#elif __sun
    #include <procfs.h>
#elif __NetBSD__
    #include <sys/types.h>
    #include <sys/sysctl.h>
#endif

static const char* getGdmVersion(FFstrbuf* version) {
    const char* error = ffProcessAppendStdOut(version, (char* const[]) { "gdm3", "--version", nullptr });
    if (error || version->length == 0) {
        error = ffProcessAppendStdOut(version, (char* const[]) { "gdm", "--version", nullptr });
        if (error || version->length == 0) {
            return "Failed to get GDM version";
        }
    }

    // GDM 44.1
    ffStrbufSubstrAfterFirstC(version, ' ');
    return nullptr;
}

static const char* getSshdVersion(FFstrbuf* version) {
    const char* error = ffProcessAppendStdErr(version, (char* const[]) { "sshd", "-V", nullptr });
    if (error) {
        return error;
    }

    // OpenSSH_9.0p1, OpenSSL 3.0.9 30 May 2023...
    ffStrbufSubstrBeforeFirstC(version, ',');
    ffStrbufSubstrAfterFirstC(version, '_');
    return nullptr;
}

#ifdef FF_HAVE_ZLIB
    #include "common/library.h"
    #include <stdlib.h>
    #include <zlib.h>

static const char* getSddmVersion(FFstrbuf* version) {
    FF_LIBRARY_LOAD_MESSAGE(zlib, "libz" FF_LIBRARY_EXTENSION, 2)
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(zlib, gzopen)
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(zlib, gzread)
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(zlib, gzerror)
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(zlib, gztell)
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(zlib, gzrewind)
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(zlib, gzclose)

    gzFile file = ffgzopen(FASTFETCH_TARGET_DIR_USR "/share/man/man1/sddm.1.gz", "rb");
    if (file == Z_NULL) {
        return "ffgzopen(\"/usr/share/man/man1/sddm.1.gz\", \"rb\") failed";
    }

    ffStrbufEnsureFree(version, 2047);
    memset(version->chars, 0, version->allocated);
    int size = ffgzread(file, version->chars, version->allocated - 1);
    ffgzclose(file);

    if (size <= 0) {
        return "ffgzread(file, version->chars, version->length) failed";
    }

    version->length = (uint32_t) size;
    uint32_t index = ffStrbufFirstIndexS(version, ".TH ");
    if (index == version->length) {
        ffStrbufClear(version);
        return ".TH is not found";
    }

    ffStrbufSubstrBefore(version, ffStrbufNextIndexC(version, index, '\n'));
    ffStrbufSubstrAfter(version, index + (uint32_t) strlen(".TH "));

    // "SDDM" 1 "May 2014" "sddm 0.20.0" "sddm"
    ffStrbufSubstrBeforeLastC(version, ' ');
    ffStrbufTrimRight(version, '"');
    ffStrbufSubstrAfterLastC(version, ' ');

    return nullptr;
}
#else
static const char* getSddmVersion([[maybe_unused]] FFstrbuf* version) {
    return "Fastfetch is built without libz support";
}
#endif

static const char* getXfwmVersion(FFstrbuf* version) {
    const char* error = ffProcessAppendStdOut(version, (char* const[]) { "xfwm4", "--version", nullptr });
    if (error) {
        return error;
    }

    // This is xfwm4 version 4.18.0 (revision 7e7473c5b) for Xfce 4.18...
    ffStrbufSubstrAfterFirstS(version, "version ");
    ffStrbufSubstrBeforeFirstC(version, ' ');

    return nullptr;
}

static const char* getLightdmVersion(FFstrbuf* version) {
    const char* error = ffProcessAppendStdErr(version, (char* const[]) { "lightdm", "--version", nullptr });
    if (error) {
        return error;
    }

    // lightdm 1.30.0
    ffStrbufSubstrAfterFirstC(version, ' ');
    ffStrbufTrimRight(version, '\n');

    return nullptr;
}

#if __linux__ && !__ANDROID__
#ifdef FF_HAVE_DBUS
static const char* detectBySystemdDbus(FFLMResult* result) {
    // This is the standard way to query the systemd session service name,
    // and requires no $XDG_SESSION_ID being available
    FF_DBUS_AUTO_DESTROY_DATA FFDBusData dbus = {};
    if (ffDBusLoadData(DBUS_BUS_SYSTEM, &dbus) != nullptr) {
        return "Failed to load system DBus";
    }

    if (!ffDBusGetPropertyString(
            &dbus,
            "org.freedesktop.login1",
            "/org/freedesktop/login1/session/auto",
            "org.freedesktop.login1.Session",
            "Service",
            &result->service)) {
        return "Failed to get systemd session Service property";
    }

    return nullptr;
}
#endif

#define FF_SYSTEMD_SESSIONS_PATH "/run/systemd/sessions/"
#define FF_SYSTEMD_USERS_PATH "/run/systemd/users/"

static const char* detectBySystemdPrivate(FFLMResult* result) {
    FF_STRBUF_AUTO_DESTROY path = ffStrbufCreate();

    FF_STRBUF_AUTO_DESTROY sessionId = ffStrbufCreateS(getenv("XDG_SESSION_ID"));
    if (sessionId.length == 0) {
        // On some incorrectly configured systems, $XDG_SESSION_ID is not set. Try finding it ourself
        // WARNING: This is private data. Do not parse
        ffStrbufAppendS(&path, FF_SYSTEMD_USERS_PATH);
        ffStrbufAppendUInt(&path, instance.state.platform.uid);

        // This is actually buggy, and assumes current user is using DE
        // `sd_pid_get_session` can be a better option, but we need to find a pid to use
        if (!ffParsePropFile(path.chars, "DISPLAY=", &sessionId)) {
            return "Failed to get $XDG_SESSION_ID";
        }
    }

    ffStrbufClear(&path);
    ffStrbufAppendS(&path, FF_SYSTEMD_SESSIONS_PATH);
    ffStrbufAppend(&path, &sessionId);

    // WARNING: This is private data. Do not parse
    if (!ffParsePropFile(path.chars, "SERVICE=", &result->service)) {
        return "Failed to parse " FF_SYSTEMD_SESSIONS_PATH "$XDG_SESSION_ID";
    }

    return nullptr;
}
#endif

static const char* testLms(const char* psName) {
    static const char* A[] = {
        "atrium",
    };
    static const char* C[] = {
        "cdm",
    };
    static const char* E[] = {
        "entrance",
    };
    static const char* G[] = {
        "gdm",
        "gdm3",
        "greetd",
    };
    static const char* L[] = {
        "lemurs",
        "lightdm",
        "lxdm",
        "ly",
    };
    static const char* P[] = {
        "plasmalogin",
    };
    static const char* S[] = {
        "sddm",
        "slim",
    };
    static const char* T[] = {
        "tbsm",
    };
    static const char* X[] = {
        "xdm",
    };

    switch (psName[0]) {
        #define PS_CASE(letter, array) \
            case letter: \
                for (uint32_t i = 0; i < ARRAY_SIZE(array); ++i) { \
                    if (ffStrEqualsIgnCase(psName, array[i])) { \
                        return array[i]; \
                    } \
                } \
                break;
        PS_CASE('a', A)
        PS_CASE('c', C)
        PS_CASE('e', E)
        PS_CASE('g', G)
        PS_CASE('l', L)
        PS_CASE('p', P)
        PS_CASE('s', S)
        PS_CASE('t', T)
        PS_CASE('x', X)
    }
    return nullptr;
}

const char* detectByProcesses(FFLMResult* result) {
#if __FreeBSD__
    #ifdef __DragonFly__
        #define ki_comm kp_comm
    #endif

    int request[] = { CTL_KERN, KERN_PROC, KERN_PROC_UID, 0 };
    size_t length = 0;

    if (sysctl(request, ARRAY_SIZE(request), nullptr, &length, nullptr, 0) != 0) {
        return "sysctl({CTL_KERN, KERN_PROC, KERN_PROC_UID}, nullptr) failed";
    }

    FF_AUTO_FREE struct kinfo_proc* procs = (struct kinfo_proc*) malloc(length);
    if (sysctl(request, ARRAY_SIZE(request), procs, &length, nullptr, 0) != 0) {
        return "sysctl({CTL_KERN, KERN_PROC, KERN_PROC_UID}, procs) failed";
    }

    length /= sizeof(*procs);

    for (struct kinfo_proc* proc = procs; proc < procs + length; ++proc) {
        const char* lm = testLms(proc->ki_comm);
        if (lm) {
            ffStrbufSetStatic(&result->service, lm);
            break;
        }
    }
#elif __OpenBSD__
    kvm_t* kd = kvm_open(nullptr, nullptr, nullptr, KVM_NO_FILES, nullptr);
    int count = 0;
    const struct kinfo_proc* proc = kvm_getprocs(kd, KERN_PROC_UID, 0, sizeof(*proc), &count);
    if (proc) {
        for (int i = 0; i < count; ++i) {
            const char* lm = testLms(proc[i].p_comm);
            if (lm) {
                ffStrbufSetStatic(&result->service, lm);
                break;
            }
        }
    }
    kvm_close(kd);
#elif __sun
    FF_AUTO_CLOSE_DIR DIR* procdir = opendir("/proc");
    if (procdir == nullptr) {
        return "opendir(\"/proc\") failed";
    }

    FF_STRBUF_AUTO_DESTROY procPath = ffStrbufCreateA(64);
    ffStrbufAppendS(&procPath, "/proc/");

    uint32_t procPathLength = procPath.length;

    struct dirent* dirent;
    while ((dirent = readdir(procdir)) != nullptr) {
        if (!ffCharIsDigit(dirent->d_name[0])) {
            continue;
        }

        ffStrbufAppendS(&procPath, dirent->d_name);
        ffStrbufAppendS(&procPath, "/psinfo");
        psinfo_t proc;
        if (ffReadFileData(procPath.chars, sizeof(proc), &proc) == sizeof(proc)) {
            ffStrbufSubstrBefore(&procPath, procPathLength);

            if (proc.pr_uid != 0) {
                continue;
            }

            const char* lm = testLms(proc.pr_fname);
            if (lm) {
                ffStrbufSetS(&result->service, lm);
                break;
            }
        }
    }
#elif __linux__ || __GNU__
    FF_AUTO_CLOSE_DIR DIR* procdir = opendir("/proc");
    if (procdir == nullptr) {
        return "opendir(\"/proc\") failed";
    }

    FF_STRBUF_AUTO_DESTROY procPath = ffStrbufCreateA(64);
    int procfd = dirfd(procdir);

    struct dirent* dirent;
    while ((dirent = readdir(procdir)) != nullptr) {
        // Match only folders starting with a number (the pid folders)
        if (dirent->d_type != DT_DIR || !ffCharIsDigit(dirent->d_name[0])) {
            continue;
        }

        ffStrbufSetS(&procPath, dirent->d_name);
        uint32_t procFolderPathLength = procPath.length;

        // Don't check for processes not owned by root (login managers run as root).
        char loginuid[32];
        ffStrbufAppendS(&procPath, "/loginuid");
        ssize_t bytesRead = ffReadFileDataRelative(procfd, procPath.chars, sizeof(loginuid) - 1, loginuid);
        if (bytesRead <= 0) {
            continue;
        }
        loginuid[bytesRead] = '\0';

        if (strtol(loginuid, nullptr, 10) != (long) (uint32_t) -1 /* no loginuid */) {
            continue;
        }

        ffStrbufSubstrBefore(&procPath, procFolderPathLength);

        ffStrbufAppendS(&procPath, "/comm");
        char comm[256];
        bytesRead = ffReadFileDataRelative(procfd, procPath.chars, sizeof(comm) - 1, comm);
        if (bytesRead <= 0) {
            continue;
        }
        if (comm[bytesRead - 1] == '\n') {
            --bytesRead;
        }
        comm[bytesRead] = '\0';

        const char* lm = testLms(comm);
        if (lm) {
            ffStrbufSetS(&result->service, lm);
            break;
        }
    }
#elif __NetBSD__
    int request[] = { CTL_KERN, KERN_PROC2, KERN_PROC_UID, 0, sizeof(struct kinfo_proc2), INT_MAX };

    size_t size = 0;
    if (sysctl(request, ARRAY_SIZE(request), nullptr, &size, nullptr, 0) != 0) {
        return "sysctl(KERN_PROC_UID, nullptr) failed";
    }

    FF_AUTO_FREE struct kinfo_proc2* procs = malloc(size);

    if (sysctl(request, ARRAY_SIZE(request), procs, &size, nullptr, 0) != 0) {
        return "sysctl(KERN_PROC_UID, procs) failed";
    }

    for (struct kinfo_proc2* proc = procs; proc < procs + (size / sizeof(struct kinfo_proc2)); proc++) {
        const char* lm = testLms(proc->p_comm);
        if (lm) {
            ffStrbufSetStatic(&result->service, lm);
            break;
        }
    }
#endif

    if (result->service.length == 0) {
        return "Failed to detect login manager by processes";
    }

    return nullptr;
}

const char* ffDetectLM(FFLMResult* result) {
    const char* error = "";
#if __linux__
    #ifdef FF_HAVE_DBUS
        error = detectBySystemdDbus(result);
    #endif
    if (error != nullptr) {
        error = detectBySystemdPrivate(result);
    }
#endif

    if (error != nullptr) {
        error = detectByProcesses(result);
    }

    if (error != nullptr) {
        return error;
    }

    if (instance.config.general.detectVersion) {
        if (ffStrbufStartsWithS(&result->service, "gdm")) {
            getGdmVersion(&result->version);
        } else if (ffStrbufStartsWithS(&result->service, "sddm")) {
            getSddmVersion(&result->version);
        } else if (ffStrbufStartsWithS(&result->service, "xfwm")) {
            getXfwmVersion(&result->version);
        } else if (ffStrbufStartsWithS(&result->service, "lightdm")) {
            getLightdmVersion(&result->version);
        } else if (ffStrbufStartsWithS(&result->service, "sshd")) {
            getSshdVersion(&result->version);
        }
    }

    return nullptr;
}
