#include "fastfetch.h"

#include <unistd.h>
#include <sys/stat.h>
#include <signal.h>

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
    ffStrbufInitA(&xdgConfigDirs, 64);
    ffStrbufAppendS(&xdgConfigDirs, getenv("XDG_CONFIG_DIRS"));

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

    uint32_t startIndex = 0;
    while (startIndex < xdgConfigDirs.length)
    {
        uint32_t colonIndex = ffStrbufNextIndexC(&xdgConfigDirs, startIndex, ':');
        xdgConfigDirs.chars[colonIndex] = '\0';

        FFstrbuf* buffer = (FFstrbuf*) ffListAdd(&state->configDirs);
        ffStrbufInitA(buffer, 64);
        ffStrbufAppendS(buffer, xdgConfigDirs.chars + startIndex);
        ffStrbufTrimRight(buffer, '/');
        FF_ENSURE_ONLY_ONCE_IN_LIST(buffer);

        startIndex = colonIndex + 1;
    }

    FFstrbuf* systemConfigHome = ffListAdd(&state->configDirs);
    ffStrbufInitA(systemConfigHome, 64);
    ffStrbufAppendS(systemConfigHome, FASTFETCH_TARGET_DIR_ROOT"/etc/xdg");
    FF_ENSURE_ONLY_ONCE_IN_LIST(systemConfigHome)

    FFstrbuf* systemConfig = ffListAdd(&state->configDirs);
    ffStrbufInitA(systemConfig, 64);
    ffStrbufAppendS(systemConfig, FASTFETCH_TARGET_DIR_ROOT"/etc");
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
    else if(!ffStrbufEndsWithC(&state->cacheDir, '/'))
        ffStrbufAppendC(&state->cacheDir, '/');

    mkdir(state->cacheDir.chars, S_IRWXU | S_IXGRP | S_IRGRP | S_IXOTH | S_IROTH); //I hope everybody has a cache folder, but who knows

    ffStrbufAppendS(&state->cacheDir, "fastfetch/");
    mkdir(state->cacheDir.chars, S_IRWXU | S_IRGRP | S_IROTH);
}

static void initState(FFstate* state)
{
    state->logoWidth = 0;
    state->logoHeight = 0;
    state->keysHeight = 0;
    state->passwd = getpwuid(getuid());
    uname(&state->utsname);
    sysinfo(&state->sysinfo);

    initConfigDirs(state);
    initCacheDir(state);
}

static void defaultConfig(FFinstance* instance)
{
    ffStrbufInit(&instance->config.logoSource);
    instance->config.logoType = FF_LOGO_TYPE_AUTO;
    for(uint8_t i = 0; i < (uint8_t) FASTFETCH_LOGO_MAX_COLORS; ++i)
        ffStrbufInit(&instance->config.logoColors[i]);
    instance->config.logoWidth = 0;
    instance->config.logoHeight = 0; //preserve aspect ratio
    instance->config.logoPaddingLeft = 0;
    instance->config.logoPaddingRight = 4;
    instance->config.logoPrintRemaining = true;

    ffStrbufInit(&instance->config.mainColor);
    ffStrbufInit(&instance->config.separator);
    ffStrbufAppendS(&instance->config.separator, ": ");

    instance->config.showErrors = false;
    instance->config.recache = false;
    instance->config.cacheSave = true;
    instance->config.allowSlowOperations = false;
    instance->config.disableLinewrap = true;
    instance->config.hideCursor = true;
    instance->config.escapeBedrock = true;
    instance->config.glType = FF_GL_TYPE_AUTO;

    ffStrbufInitA(&instance->config.osFormat, 0);
    ffStrbufInitA(&instance->config.osKey, 0);
    ffStrbufInitA(&instance->config.hostFormat, 0);
    ffStrbufInitA(&instance->config.hostKey, 0);
    ffStrbufInitA(&instance->config.kernelFormat, 0);
    ffStrbufInitA(&instance->config.kernelKey, 0);
    ffStrbufInitA(&instance->config.uptimeFormat, 0);
    ffStrbufInitA(&instance->config.uptimeKey, 0);
    ffStrbufInitA(&instance->config.processesFormat, 0);
    ffStrbufInitA(&instance->config.processesKey, 0);
    ffStrbufInitA(&instance->config.packagesFormat, 0);
    ffStrbufInitA(&instance->config.packagesKey, 0);
    ffStrbufInitA(&instance->config.shellFormat, 0);
    ffStrbufInitA(&instance->config.shellKey, 0);
    ffStrbufInitA(&instance->config.resolutionFormat, 0);
    ffStrbufInitA(&instance->config.resolutionKey, 0);
    ffStrbufInitA(&instance->config.deFormat, 0);
    ffStrbufInitA(&instance->config.deKey, 0);
    ffStrbufInitA(&instance->config.wmFormat, 0);
    ffStrbufInitA(&instance->config.wmKey, 0);
    ffStrbufInitA(&instance->config.wmThemeFormat, 0);
    ffStrbufInitA(&instance->config.wmThemeKey, 0);
    ffStrbufInitA(&instance->config.themeFormat, 0);
    ffStrbufInitA(&instance->config.themeKey, 0);
    ffStrbufInitA(&instance->config.iconsFormat, 0);
    ffStrbufInitA(&instance->config.iconsKey, 0);
    ffStrbufInitA(&instance->config.fontFormat, 0);
    ffStrbufInitA(&instance->config.fontKey, 0);
    ffStrbufInitA(&instance->config.cursorKey, 0);
    ffStrbufInitA(&instance->config.cursorFormat, 0);
    ffStrbufInitA(&instance->config.terminalFormat, 0);
    ffStrbufInitA(&instance->config.terminalKey, 0);
    ffStrbufInitA(&instance->config.termFontFormat, 0);
    ffStrbufInitA(&instance->config.termFontKey, 0);
    ffStrbufInitA(&instance->config.cpuFormat, 0);
    ffStrbufInitA(&instance->config.cpuKey, 0);
    ffStrbufInitA(&instance->config.cpuUsageFormat, 0);
    ffStrbufInitA(&instance->config.cpuUsageKey, 0);
    ffStrbufInitA(&instance->config.gpuFormat, 0);
    ffStrbufInitA(&instance->config.gpuKey, 0);
    ffStrbufInitA(&instance->config.memoryFormat, 0);
    ffStrbufInitA(&instance->config.memoryKey, 0);
    ffStrbufInitA(&instance->config.diskFormat, 0);
    ffStrbufInitA(&instance->config.diskKey, 0);
    ffStrbufInitA(&instance->config.batteryFormat, 0);
    ffStrbufInitA(&instance->config.batteryKey, 0);
    ffStrbufInitA(&instance->config.localeFormat, 0);
    ffStrbufInitA(&instance->config.localeKey, 0);
    ffStrbufInitA(&instance->config.localIpKey, 0);
    ffStrbufInitA(&instance->config.localIpFormat, 0);
    ffStrbufInitA(&instance->config.publicIpKey, 0);
    ffStrbufInitA(&instance->config.publicIpFormat, 0);
    ffStrbufInitA(&instance->config.playerKey, 0);
    ffStrbufInitA(&instance->config.playerFormat, 0);
    ffStrbufInitA(&instance->config.songKey, 0);
    ffStrbufInitA(&instance->config.songFormat, 0);
    ffStrbufInitA(&instance->config.dateTimeKey, 0);
    ffStrbufInitA(&instance->config.dateTimeFormat, 0);
    ffStrbufInitA(&instance->config.dateKey, 0);
    ffStrbufInitA(&instance->config.dateFormat, 0);
    ffStrbufInitA(&instance->config.timeKey, 0);
    ffStrbufInitA(&instance->config.timeFormat, 0);
    ffStrbufInitA(&instance->config.vulkanKey, 0);
    ffStrbufInitA(&instance->config.vulkanFormat, 0);
    ffStrbufInitA(&instance->config.openGLKey, 0);
    ffStrbufInitA(&instance->config.openGLFormat, 0);
    ffStrbufInitA(&instance->config.openCLKey, 0);
    ffStrbufInitA(&instance->config.openCLFormat, 0);

    ffStrbufInitA(&instance->config.libPCI, 0);
    ffStrbufInitA(&instance->config.libVulkan, 0);
    ffStrbufInitA(&instance->config.libWayland, 0);
    ffStrbufInitA(&instance->config.libXcbRandr, 0);
    ffStrbufInitA(&instance->config.libXcb, 0);
    ffStrbufInitA(&instance->config.libXrandr, 0);
    ffStrbufInitA(&instance->config.libX11, 0);
    ffStrbufInitA(&instance->config.libGIO, 0);
    ffStrbufInitA(&instance->config.libDConf, 0);
    ffStrbufInitA(&instance->config.libDBus, 0);
    ffStrbufInitA(&instance->config.libXFConf, 0);
    ffStrbufInitA(&instance->config.libSQLite3, 0);
    ffStrbufInitA(&instance->config.librpm, 0);
    ffStrbufInitA(&instance->config.libImageMagick, 0);
    ffStrbufInitA(&instance->config.libZ, 0);
    ffStrbufInitA(&instance->config.libChafa, 0);
    ffStrbufInitA(&instance->config.libEGL, 0);
    ffStrbufInitA(&instance->config.libGLX, 0);
    ffStrbufInitA(&instance->config.libOSMesa, 0);
    ffStrbufInitA(&instance->config.libOpenCL, 0);

    ffStrbufInitA(&instance->config.diskFolders, 0);

    ffStrbufInitA(&instance->config.batteryDir, 0);

    ffStrbufInitA(&instance->config.separatorString, 0);

    instance->config.localIpShowIpV4 = true;
    instance->config.localIpShowIpV6 = false;
    instance->config.localIpShowLoop = false;

    instance->config.publicIpTimeout = 0;

    ffStrbufInitA(&instance->config.osFile, 0);

    ffStrbufInitA(&instance->config.playerName, 0);
}

void ffInitInstance(FFinstance* instance)
{
    initState(&instance->state);
    defaultConfig(instance);
}

static void resetConsole(bool disableLinewrap, bool hideCursor)
{
    if(disableLinewrap)
        fputs("\033[?7h", stdout);

    if(hideCursor)
        fputs("\033[?25h", stdout);
}

static volatile bool ffDisableLinewrap = true;
static volatile bool ffHideCursor = true;

static void exitSignalHandler(int signal)
{
    FF_UNUSED(signal);
    resetConsole(ffDisableLinewrap, ffHideCursor);
    exit(0);
}

void ffStart(FFinstance* instance)
{
    ffDisableLinewrap = instance->config.disableLinewrap;
    ffHideCursor = instance->config.hideCursor;

    struct sigaction action = {};
    action.sa_handler = exitSignalHandler;

    sigaction(SIGINT, &action, NULL);
    sigaction(SIGTERM, &action, NULL);
    sigaction(SIGQUIT, &action, NULL);

    //We do the cache validation here, so we can skip it if --recache is given
    ffCacheValidate(instance);

    //reset everything to default before we start printing
    fputs(FASTFETCH_TEXT_MODIFIER_RESET, stdout);

    if(instance->config.hideCursor)
        fputs("\033[?25l", stdout);

    if(instance->config.disableLinewrap)
        fputs("\033[?7l", stdout);

    ffPrintLogo(instance);
}

void ffFinish(FFinstance* instance)
{
    ffPrintRemainingLogo(instance);
    resetConsole(instance->config.disableLinewrap, instance->config.hideCursor);
}

//Must be in a file compiled with the libfastfetch target, because the FF_HAVE* macros are not defined for the executable targets
void ffListFeatures()
{
    fputs(
        #ifdef FF_HAVE_LIBPCI
            "libpci\n"
        #endif
        #ifdef FF_HAVE_VULKAN
            "vulkan\n"
        #endif
        #ifdef FF_HAVE_WAYLAND
            "wayland\n"
        #endif
        #ifdef FF_HAVE_XCB_RANDR
            "xcb-randr\n"
        #endif
        #ifdef FF_HAVE_XCB
            "xcb\n"
        #endif
        #ifdef FF_HAVE_XRANDR
            "xrandr\n"
        #endif
        #ifdef FF_HAVE_X11
            "x11\n"
        #endif
        #ifdef FF_HAVE_GIO
            "gio\n"
        #endif
        #ifdef FF_HAVE_DCONF
            "dconf\n"
        #endif
        #ifdef FF_HAVE_DBUS
            "dbus\n"
        #endif
        #ifdef FF_HAVE_IMAGEMAGICK7
            "imagemagick7\n"
        #endif
        #ifdef FF_HAVE_IMAGEMAGICK6
            "imagemagick6\n"
        #endif
        #ifdef FF_HAVE_CHAFA
            "chafa\n"
        #endif
        #ifdef FF_HAVE_ZLIB
            "zlib\n"
        #endif
        #ifdef FF_HAVE_XFCONF
            "xfconf\n"
        #endif
        #ifdef FF_HAVE_SQLITE3
            "sqlite3\n"
        #endif
        #ifdef FF_HAVE_RPM
            "rpm\n"
        #endif
        #ifdef FF_HAVE_EGL
            "egl\n"
        #endif
        #ifdef FF_HAVE_GLX
            "glx\n"
        #endif
        #ifdef FF_HAVE_OSMESA
            "osmesa\n"
        #endif
        #ifdef FF_HAVE_OPENCL
            "opencl\n"
        #endif
        ""
    , stdout);
}
