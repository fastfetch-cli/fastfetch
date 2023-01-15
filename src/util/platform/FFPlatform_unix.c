#include "FFPlatform_private.h"

#include "fastfetch_config.h"

#include <unistd.h>
#include <pwd.h>
#include <sys/utsname.h>
#include <netdb.h>

static void getHomeDir(FFPlatform* platform, const struct passwd* pwd)
{
    const char* home = getenv("HOME");
    if(!ffStrSet(home) && pwd)
        home = pwd->pw_dir;

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

    ffStrbufAppendS(&platform->cacheDir, "fastfetch/");
}

static void getConfigDirs(FFPlatform* platform)
{
    ffPlatformPathAddEnv(&platform->configDirs, "XDG_CONFIG_HOME");
    ffPlatformPathAddHome(&platform->configDirs, platform, ".config/");

    #if defined(__APPLE__)
        pathsAddHome(&state->configDirs, state, "/Library/Preferences/");
    #endif

    ffPlatformPathAddHome(&platform->configDirs, platform, "");
    ffPlatformPathAddEnv(&platform->configDirs, "XDG_CONFIG_DIRS");

    #if !defined(__APPLE__)
        ffPlatformPathAddAbsolute(&platform->configDirs, FASTFETCH_TARGET_DIR_ETC"/xdg/");
    #endif

    ffPlatformPathAddAbsolute(&platform->configDirs, FASTFETCH_TARGET_DIR_ETC"/");
    ffPlatformPathAddAbsolute(&platform->configDirs, FASTFETCH_TARGET_DIR_INSTALL_SYSCONF"/");
}

static void getDataDirs(FFPlatform* platform)
{
    ffPlatformPathAddEnv(&platform->dataDirs, "XDG_DATA_HOME");
    ffPlatformPathAddHome(&platform->dataDirs, platform, ".local/share/");
    ffPlatformPathAddHome(&platform->dataDirs, platform, "");
    ffPlatformPathAddEnv(&platform->dataDirs, "XDG_DATA_DIRS");
    ffPlatformPathAddAbsolute(&platform->dataDirs, FASTFETCH_TARGET_DIR_USR"/local/share/");
    ffPlatformPathAddAbsolute(&platform->dataDirs, FASTFETCH_TARGET_DIR_USR"/share/");
}

static void getUserName(FFPlatform* platform, const struct passwd* pwd)
{
    const char* user = getenv("USER");
    if(!ffStrSet(user) && pwd)
        user = pwd->pw_name;

    ffStrbufAppendS(&platform->userName, user);
}

static void getHostName(FFPlatform* platform, const struct utsname* uts)
{
    char hostname[256];
    if(gethostname(hostname, sizeof(hostname)) == 0)
        ffStrbufAppendS(&platform->hostName, hostname);

    if(platform->hostName.length == 0)
        ffStrbufAppendS(&platform->hostName, uts->nodename);
}

static void getDomainName(FFPlatform* platform)
{
    struct addrinfo hints = {0};
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_CANONNAME;

    struct addrinfo* info = NULL;

    if(getaddrinfo(platform->hostName.chars, "80", &hints, &info) != 0)
        return;

    struct addrinfo* current = info;
    while(platform->domainName.length == 0 && current != NULL)
    {
        ffStrbufAppendS(&platform->domainName, current->ai_canonname);
        current = current->ai_next;
    }

    freeaddrinfo(info);
}

static void getUserShell(FFPlatform* platform, const struct passwd* pwd)
{
    const char* shell = getenv("SHELL");
    if(!ffStrSet(shell) && pwd)
        shell = pwd->pw_shell;

    ffStrbufAppendS(&platform->userShell, shell);
}

void ffPlatformInitImpl(FFPlatform* platform)
{
    struct passwd* pwd = getpwuid(getuid());

    struct utsname uts;
    if(uname(&uts) != 0)
        memset(&uts, 0, sizeof(uts));

    getHomeDir(platform, pwd);
    getCacheDir(platform);
    getConfigDirs(platform);
    getDataDirs(platform);

    getUserName(platform, pwd);
    getHostName(platform, &uts);
    getDomainName(platform);
    getUserShell(platform, pwd);

    ffStrbufAppendS(&platform->systemName, uts.sysname);
    ffStrbufAppendS(&platform->systemRelease, uts.release);
    ffStrbufAppendS(&platform->systemVersion, uts.version);
    ffStrbufAppendS(&platform->systemArchitecture, uts.machine);
}
