#include "fastfetch.h"

#include <unistd.h>
#include <sys/stat.h>

static bool strbufEqualsAdapter(const void* first, const void* second)
{
    return ffStrbufComp(second, first) == 0;
}

static void initConfigDirs(FFstate* state)
{
    ffListInit(&state->configDirs, sizeof(FFstrbuf));

    const char* xdgConfigHome = getenv("XDG_CONFIG_HOME");
    if(xdgConfigHome != NULL)
    {
        FFstrbuf* buffer = (FFstrbuf*) ffListAdd(&state->configDirs);
        ffStrbufInitA(buffer, 64);
        ffStrbufAppendS(buffer, xdgConfigHome);
        ffStrbufTrimRight(buffer, '/');
    }

    FFstrbuf xdgConfigDirs;
    ffStrbufInitAS(&xdgConfigDirs, 64, getenv("XDG_CONFIG_DIRS"));
    uint32_t lastIndex = 0;
    while (lastIndex < xdgConfigDirs.length)
    {
        uint32_t colonIndex = ffStrbufFirstIndexAfterC(&xdgConfigDirs, lastIndex, ':');
        xdgConfigDirs.chars[colonIndex] = '\0';

        FFstrbuf* buffer = (FFstrbuf*) ffListAdd(&state->configDirs);
        ffStrbufInitA(buffer, 64);
        ffStrbufAppendS(buffer, xdgConfigDirs.chars + lastIndex);
        ffStrbufTrimRight(buffer, '/');

        lastIndex = colonIndex + 1;
    }

    #define FF_ENSURE_ONLY_ONCE_IN_LIST(element) \
        if(ffListFirstIndexComp(&state->configDirs, element, strbufEqualsAdapter) < state->configDirs.length - 1) \
            --state->configDirs.length;

    FFstrbuf* userConfigHome = ffListAdd(&state->configDirs);
    ffStrbufInitA(userConfigHome, 64);
    ffStrbufAppendS(userConfigHome, state->passwd->pw_dir);
    ffStrbufAppendS(userConfigHome, "/.config");
    FF_ENSURE_ONLY_ONCE_IN_LIST(userConfigHome)

    FFstrbuf* userHome = ffListAdd(&state->configDirs);
    ffStrbufInitA(userHome, 64);
    ffStrbufAppendS(userHome, state->passwd->pw_dir);
    FF_ENSURE_ONLY_ONCE_IN_LIST(userHome)

    FFstrbuf* systemConfigHome = ffListAdd(&state->configDirs);
    ffStrbufInitA(systemConfigHome, 64);
    ffStrbufAppendS(systemConfigHome, "/etc/xdg");
    FF_ENSURE_ONLY_ONCE_IN_LIST(systemConfigHome)

    FFstrbuf* systemConfig = ffListAdd(&state->configDirs);
    ffStrbufInitA(systemConfig, 64);
    ffStrbufAppendS(systemConfig, "/etc");
    FF_ENSURE_ONLY_ONCE_IN_LIST(systemConfig)

    #undef FF_ENSURE_ONLY_ONCE_IN_LIST
}

static void initCacheDir(FFstate* state)
{
    ffStrbufInitA(&state->cacheDir, 64);

    ffStrbufAppendS(&state->cacheDir, getenv("XDG_CACHE_HOME"));

    if(state->cacheDir.length == 0)
    {
        ffStrbufAppendS(&state->cacheDir, state->passwd->pw_dir);
        ffStrbufAppendS(&state->cacheDir, "/.cache/");
    }
    else if(state->cacheDir.chars[state->cacheDir.length - 1] != '/')
        ffStrbufAppendC(&state->cacheDir, '/');

    mkdir(state->cacheDir.chars, S_IRWXU | S_IXGRP | S_IRGRP | S_IXOTH | S_IROTH); //I hope everybody has a cache folder, but who knows

    ffStrbufAppendS(&state->cacheDir, "fastfetch/");
    mkdir(state->cacheDir.chars, S_IRWXU | S_IRGRP | S_IROTH);
}

static void initState(FFstate* state)
{
    state->logoWidth = 0;
    state->passwd = getpwuid(getuid());
    uname(&state->utsname);
    sysinfo(&state->sysinfo);

    initConfigDirs(state);
    initCacheDir(state);
}

static void defaultConfig(FFinstance* instance)
{
    ffStrbufInit(&instance->config.color);
    instance->config.logoKeySpacing = 4;
    ffStrbufInitS(&instance->config.separator, ": ");
    instance->config.offsetx = 0;
    instance->config.titleLength = 20; // This is overwritten by ffPrintTitle
    instance->config.colorLogo = true;
    instance->config.showErrors = false;
    instance->config.recache = false;
    instance->config.cacheSave = true;
    instance->config.printRemainingLogo = true;
    instance->config.allowSlowOperations = false;

    //This is basically the none logo
    for(uint8_t i = 0; i < sizeof(instance->config.logo.colors) / sizeof(instance->config.logo.colors[0]); ++i)
        instance->config.logo.colors[i] = "";
    instance->config.logo.allLinesSameLength = true;
    instance->config.logo.freeable = false;
    instance->config.logo.lines = "";

    //Since most of these properties are unlikely to be used at once, give them minimal heap space (the \0 character)
    ffStrbufInitA(&instance->config.osFormat, 1);
    ffStrbufInitA(&instance->config.osKey, 1);
    ffStrbufInitA(&instance->config.hostFormat, 1);
    ffStrbufInitA(&instance->config.hostKey, 1);
    ffStrbufInitA(&instance->config.kernelFormat, 1);
    ffStrbufInitA(&instance->config.kernelKey, 1);
    ffStrbufInitA(&instance->config.uptimeFormat, 1);
    ffStrbufInitA(&instance->config.uptimeKey, 1);
    ffStrbufInitA(&instance->config.packagesFormat, 1);
    ffStrbufInitA(&instance->config.packagesKey, 1);
    ffStrbufInitA(&instance->config.shellFormat, 1);
    ffStrbufInitA(&instance->config.shellKey, 1);
    ffStrbufInitA(&instance->config.resolutionFormat, 1);
    ffStrbufInitA(&instance->config.resolutionKey, 1);
    ffStrbufInitA(&instance->config.deFormat, 1);
    ffStrbufInitA(&instance->config.deKey, 1);
    ffStrbufInitA(&instance->config.wmFormat, 1);
    ffStrbufInitA(&instance->config.wmKey, 1);
    ffStrbufInitA(&instance->config.wmThemeFormat, 1);
    ffStrbufInitA(&instance->config.wmThemeKey, 1);
    ffStrbufInitA(&instance->config.themeFormat, 1);
    ffStrbufInitA(&instance->config.themeKey, 1);
    ffStrbufInitA(&instance->config.iconsFormat, 1);
    ffStrbufInitA(&instance->config.iconsKey, 1);
    ffStrbufInitA(&instance->config.fontFormat, 1);
    ffStrbufInitA(&instance->config.fontKey, 1);
    ffStrbufInitA(&instance->config.terminalFormat, 1);
    ffStrbufInitA(&instance->config.terminalKey, 1);
    ffStrbufInitA(&instance->config.termFontFormat, 1);
    ffStrbufInitA(&instance->config.termFontKey, 1);
    ffStrbufInitA(&instance->config.cpuFormat, 1);
    ffStrbufInitA(&instance->config.cpuKey, 1);
    ffStrbufInitA(&instance->config.gpuFormat, 1);
    ffStrbufInitA(&instance->config.gpuKey, 1);
    ffStrbufInitA(&instance->config.memoryFormat, 1);
    ffStrbufInitA(&instance->config.memoryKey, 1);
    ffStrbufInitA(&instance->config.diskFormat, 1);
    ffStrbufInitA(&instance->config.diskKey, 1);
    ffStrbufInitA(&instance->config.batteryFormat, 1);
    ffStrbufInitA(&instance->config.batteryKey, 1);
    ffStrbufInitA(&instance->config.localeFormat, 1);
    ffStrbufInitA(&instance->config.localeKey, 1);

    ffStrbufInitA(&instance->config.libPCI, 1);
    ffStrbufInitA(&instance->config.libX11, 1);
    ffStrbufInitA(&instance->config.libXrandr, 1);
    ffStrbufInitA(&instance->config.libGIO, 1);
    ffStrbufInitA(&instance->config.libDConf, 1);
    ffStrbufInitA(&instance->config.libWayland, 1);
    ffStrbufInitA(&instance->config.libXFConf, 1);
    ffStrbufInitA(&instance->config.libSQLite, 1);

    ffStrbufInitA(&instance->config.diskFolders, 1);

    ffStrbufInitA(&instance->config.batteryDir, 1);
}

void ffInitInstance(FFinstance* instance)
{
    initState(&instance->state);
    defaultConfig(instance);
    ffCacheValidate(instance);
}

static void ffCleanup(FFinstance* instance)
{
    UNUSED(instance);
    // Place for cleaning up
    // I dont destroy the global strbufs, because the OS is typically faster doing it after the program terminates
    // This eliminates one function call per strbuf + setting things like length to 0
}

void ffFinish(FFinstance* instance)
{
    if(instance->config.printRemainingLogo)
        ffPrintRemainingLogo(instance);

    ffCleanup(instance);
}
