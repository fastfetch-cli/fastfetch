#include "FFPlatform_private.h"
#include "util/FFstrbuf.h"
#include "util/stringUtils.h"
#include "fastfetch_config.h"
#include "common/io/io.h"

#include <unistd.h>
#include <pwd.h>
#include <limits.h>
#include <sys/utsname.h>
#include <paths.h>

#ifdef __APPLE__
    #include <libproc.h>
    #include <sys/sysctl.h>
#elif defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__)
    #include <sys/sysctl.h>
#elif defined(__HAIKU__)
    #include <image.h>
    #include <OS.h>
#endif

static void getExePath(FFPlatform* platform)
{
    char exePath[PATH_MAX + 1];
    #if defined(__linux__) || defined (__GNU__)
        ssize_t exePathLen = readlink("/proc/self/exe", exePath, sizeof(exePath) - 1);
        if (exePathLen >= 0)
            exePath[exePathLen] = '\0';
    #elif defined(__APPLE__)
        int exePathLen = proc_pidpath((int) getpid(), exePath, sizeof(exePath));
    #elif defined(__FreeBSD__) || defined(__NetBSD__)
        size_t exePathLen = sizeof(exePath);
        if(sysctl(
            (int[]){CTL_KERN,
            #ifdef __FreeBSD__
                KERN_PROC, KERN_PROC_PATHNAME, (int) getpid()
            #else
                KERN_PROC_ARGS, (int) getpid(), KERN_PROC_PATHNAME
            #endif
            }, 4,
            exePath, &exePathLen,
            NULL, 0
        ) < 0)
            exePathLen = 0;
        else
            exePathLen--; // remove terminating NUL
    #elif defined(__OpenBSD__)
        size_t exePathLen = 0;
    #elif defined(__sun)
        ssize_t exePathLen = readlink("/proc/self/path/a.out", exePath, sizeof(exePath) - 1);
        if (exePathLen >= 0)
            exePath[exePathLen] = '\0';
    #elif defined(__HAIKU__)
        size_t exePathLen = 0;
        image_info info;
        int32 cookie = 0;

        while (get_next_image_info(B_CURRENT_TEAM, &cookie, &info) == B_OK) {
            if (info.type == B_APP_IMAGE) {
                exePathLen = strlcpy(exePath, info.name, PATH_MAX);
                break;
            }
        }
    #endif
    if (exePathLen > 0)
    {
        ffStrbufEnsureFree(&platform->exePath, PATH_MAX);
        if (realpath(exePath, platform->exePath.chars))
            ffStrbufRecalculateLength(&platform->exePath);
        else
            ffStrbufSetNS(&platform->exePath, (uint32_t) exePathLen, exePath);
    }
}

static void platformPathAddEnv(FFlist* dirs, const char* env)
{
    const char* envValue = getenv(env);
    if(!ffStrSet(envValue))
        return;

    FF_STRBUF_AUTO_DESTROY value = ffStrbufCreateA(64);
    ffStrbufAppendS(&value, envValue);

    uint32_t startIndex = 0;
    while (startIndex < value.length)
    {
        uint32_t colonIndex = ffStrbufNextIndexC(&value, startIndex, ':');
        value.chars[colonIndex] = '\0';

        if(!ffStrSet(value.chars + startIndex))
        {
            startIndex = colonIndex + 1;
            continue;
        }

        ffPlatformPathAddAbsolute(dirs, value.chars + startIndex);

        startIndex = colonIndex + 1;
    }
}

static void getHomeDir(FFPlatform* platform, const struct passwd* pwd)
{
    const char* home = pwd ? pwd->pw_dir : getenv("HOME");
    ffStrbufAppendS(&platform->homeDir, home);
    ffStrbufEnsureEndsWithC(&platform->homeDir, '/');
}

static void getCacheDir(FFPlatform* platform)
{
    const char* cache = getenv("XDG_CACHE_HOME");
    if(ffStrSet(cache))
    {
        ffStrbufAppendS(&platform->cacheDir, cache);
        ffStrbufEnsureEndsWithC(&platform->cacheDir, '/');
    }
    else
    {
        ffStrbufAppend(&platform->cacheDir, &platform->homeDir);
        ffStrbufAppendS(&platform->cacheDir, ".cache/");
    }
}

static void getConfigDirs(FFPlatform* platform)
{
    // Always make sure `${XDG_CONFIG_HOME:-$HOME/.config}` is the first entry
    platformPathAddEnv(&platform->configDirs, "XDG_CONFIG_HOME");
    ffPlatformPathAddHome(&platform->configDirs, platform, ".config/");

    #if defined(__APPLE__)
        ffPlatformPathAddHome(&platform->configDirs, platform, "Library/Preferences/");
        ffPlatformPathAddHome(&platform->configDirs, platform, "Library/Application Support/");
    #endif
    #if defined(__HAIKU__)
        ffPlatformPathAddHome(&platform->configDirs, platform, "config/settings/");
    #endif

    ffPlatformPathAddHome(&platform->configDirs, platform, "");
    platformPathAddEnv(&platform->configDirs, "XDG_CONFIG_DIRS");

    #if !defined(__APPLE__)
        ffPlatformPathAddAbsolute(&platform->configDirs, FASTFETCH_TARGET_DIR_ETC "/xdg/");
    #endif

    ffPlatformPathAddAbsolute(&platform->configDirs, FASTFETCH_TARGET_DIR_ETC "/");
    ffPlatformPathAddAbsolute(&platform->configDirs, FASTFETCH_TARGET_DIR_INSTALL_SYSCONF "/");
}

static void getDataDirs(FFPlatform* platform)
{
    platformPathAddEnv(&platform->dataDirs, "XDG_DATA_HOME");
    ffPlatformPathAddHome(&platform->dataDirs, platform, ".local/share/");

    // Add ${currentExePath}/../share
    if (platform->exePath.length > 0)
    {
        FF_STRBUF_AUTO_DESTROY path = ffStrbufCreateCopy(&platform->exePath);
        ffStrbufSubstrBeforeLastC(&path, '/');
        ffStrbufSubstrBeforeLastC(&path, '/');
        ffStrbufAppendS(&path, "/share");
        ffPlatformPathAddAbsolute(&platform->dataDirs, path.chars);
    }

    #ifdef __APPLE__
        ffPlatformPathAddHome(&platform->dataDirs, platform, "Library/Application Support/");
    #endif

    ffPlatformPathAddHome(&platform->dataDirs, platform, "");
    platformPathAddEnv(&platform->dataDirs, "XDG_DATA_DIRS");
#ifdef _PATH_LOCALBASE
    ffPlatformPathAddAbsolute(&platform->dataDirs, _PATH_LOCALBASE "/share/");
#endif
    ffPlatformPathAddAbsolute(&platform->dataDirs, FASTFETCH_TARGET_DIR_USR "/local/share/");
    ffPlatformPathAddAbsolute(&platform->dataDirs, FASTFETCH_TARGET_DIR_USR "/share/");
}

static void getUserName(FFPlatform* platform, const struct passwd* pwd)
{
    const char* user = getenv("USER");
    if(!ffStrSet(user) && pwd)
        user = pwd->pw_name;

    ffStrbufAppendS(&platform->userName, user);

    if (pwd)
    {
        ffStrbufAppendS(&platform->fullUserName, pwd->pw_gecos);
        ffStrbufTrimSpace(&platform->fullUserName);
    }
}

static void getHostName(FFPlatform* platform, const struct utsname* uts)
{
    ffStrbufAppendS(&platform->hostName, uts->nodename);
}

static void getUserShell(FFPlatform* platform, const struct passwd* pwd)
{
    const char* shell = getenv("SHELL");
    if(!ffStrSet(shell) && pwd)
        shell = pwd->pw_shell;

    ffStrbufAppendS(&platform->userShell, shell);
}

static void getSysinfo(FFPlatformSysinfo* info, const struct utsname* uts)
{
    ffStrbufAppendS(&info->name, uts->sysname);
    ffStrbufAppendS(&info->release, uts->release);
    ffStrbufAppendS(&info->version, uts->version);
    #ifdef __HAIKU__
    /* historical reason */
    if (ffStrEquals(uts->machine, "BePC"))
        ffStrbufSetStatic(&info->architecture, "i386");
    else
    #endif
    ffStrbufAppendS(&info->architecture, uts->machine);

    #if defined(__FreeBSD__) || defined(__APPLE__) || defined(__OpenBSD__) || defined(__NetBSD__)
    size_t length = sizeof(info->pageSize);
    sysctl((int[]){ CTL_HW, HW_PAGESIZE }, 2, &info->pageSize, &length, NULL, 0);
    #else
    info->pageSize = (uint32_t) sysconf(_SC_PAGESIZE);
    #endif
}

void ffPlatformInitImpl(FFPlatform* platform)
{
    struct passwd* pwd = getpwuid(getuid());

    struct utsname uts;
    if(uname(&uts) < 0)
        memset(&uts, 0, sizeof(uts));

    getExePath(platform);
    getHomeDir(platform, pwd);
    getCacheDir(platform);
    getConfigDirs(platform);
    getDataDirs(platform);

    getUserName(platform, pwd);
    getHostName(platform, &uts);
    getUserShell(platform, pwd);

    getSysinfo(&platform->sysinfo, &uts);
}
