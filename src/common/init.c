#include "fastfetch.h"
#include "common/parsing.h"
#include "common/thread.h"
#include "detection/displayserver/displayserver.h"
#include "detection/terminaltheme/terminaltheme.h"
#include "util/textModifier.h"
#include "logo/logo.h"

#include <stdlib.h>
#include <unistd.h>
#include <locale.h>
#ifdef _WIN32
    #include <windows.h>
    #include "util/windows/unicode.h"
#else
    #include <signal.h>
#endif

FFinstance instance; // Global singleton

static void initState(FFstate* state)
{
    state->logoWidth = 0;
    state->logoHeight = 0;
    state->keysHeight = 0;
    state->terminalLightTheme = false;

    ffPlatformInit(&state->platform);
    state->configDoc = NULL;
    state->resultDoc = NULL;

    {
        // don't enable bright color if the terminal is in light mode
        FFTerminalThemeResult result;
        if (ffDetectTerminalTheme(&result, true /* forceEnv for performance */) && !result.bg.dark)
            state->terminalLightTheme = true;
    }
}

static void defaultConfig(void)
{
    ffOptionsInitLogo(&instance.config.logo);
    ffOptionsInitGeneral(&instance.config.general);
    ffOptionsInitModules(&instance.config.modules);
    ffOptionsInitDisplay(&instance.config.display);
}

void ffInitInstance(void)
{
    #ifdef WIN32
        //https://learn.microsoft.com/en-us/cpp/c-runtime-library/reference/setlocale-wsetlocale?source=recommendations&view=msvc-170#utf-8-support
        setlocale(LC_ALL, ".UTF8");
    #else
        // Never use `setlocale(LC_ALL, "")`
        setlocale(LC_TIME, "");
    #endif

    initState(&instance.state);
    defaultConfig();
}

static volatile bool ffDisableLinewrap = true;
static volatile bool ffHideCursor = true;

static void resetConsole(void)
{
    if(ffDisableLinewrap)
        fputs("\033[?7h", stdout);

    if(ffHideCursor)
        fputs("\033[?25h", stdout);

    #if defined(_WIN32)
        fflush(stdout);
    #endif
}

#ifdef _WIN32
BOOL WINAPI consoleHandler(DWORD signal)
{
    FF_UNUSED(signal);
    resetConsole();
    exit(0);
}
#else
static void exitSignalHandler(int signal)
{
    FF_UNUSED(signal);
    resetConsole();
    exit(0);
}
#endif

void ffStart(void)
{
    #ifdef FF_START_DETECTION_THREADS
        if(instance.config.general.multithreading)
            startDetectionThreads();
    #endif

    ffDisableLinewrap = instance.config.display.disableLinewrap && !instance.config.display.pipe && !instance.state.resultDoc;
    ffHideCursor = instance.config.display.hideCursor && !instance.config.display.pipe && !instance.state.resultDoc;

    #ifdef _WIN32
    if (instance.config.display.noBuffer)
        setvbuf(stdout, NULL, _IONBF, 0);
    else
        setvbuf(stdout, NULL, _IOFBF, 4096);
    SetConsoleCtrlHandler(consoleHandler, TRUE);
    HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD mode = 0;
    GetConsoleMode(hStdout, &mode);
    SetConsoleMode(hStdout, mode | ENABLE_PROCESSED_OUTPUT | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
    SetConsoleOutputCP(CP_UTF8);
    #else
    if (instance.config.display.noBuffer) setvbuf(stdout, NULL, _IONBF, 0);
    struct sigaction action = { .sa_handler = exitSignalHandler };
    sigaction(SIGINT, &action, NULL);
    sigaction(SIGTERM, &action, NULL);
    sigaction(SIGQUIT, &action, NULL);
    #endif

    //reset everything to default before we start printing
    if(!instance.config.display.pipe && !instance.state.resultDoc)
        fputs(FASTFETCH_TEXT_MODIFIER_RESET, stdout);

    if(ffHideCursor)
        fputs("\033[?25l", stdout);

    if(ffDisableLinewrap)
        fputs("\033[?7l", stdout);

    ffLogoPrint();
}

void ffFinish(void)
{
    if(instance.config.logo.printRemaining)
        ffLogoPrintRemaining();

    resetConsole();
}

static void destroyConfig(void)
{
    ffOptionsDestroyLogo(&instance.config.logo);
    ffOptionsDestroyGeneral(&instance.config.general);
    ffOptionsDestroyModules(&instance.config.modules);
    ffOptionsDestroyDisplay(&instance.config.display);
}

static void destroyState(void)
{
    ffPlatformDestroy(&instance.state.platform);
    yyjson_doc_free(instance.state.configDoc);
    yyjson_mut_doc_free(instance.state.resultDoc);
    ffStrbufDestroy(&instance.state.genConfigPath);
}

void ffDestroyInstance(void)
{
    destroyConfig();
    destroyState();
}

//Must be in a file compiled with the libfastfetch target, because the FF_HAVE* macros are not defined for the executable targets
void ffListFeatures(void)
{
    fputs(
        #if FF_HAVE_THREADS
            "threads\n"
        #endif
        #if FF_HAVE_VULKAN
            "vulkan\n"
        #endif
        #if FF_HAVE_WAYLAND
            "wayland\n"
        #endif
        #if FF_HAVE_XCB_RANDR
            "xcb-randr\n"
        #endif
        #if FF_HAVE_XCB
            "xcb\n"
        #endif
        #if FF_HAVE_XRANDR
            "xrandr\n"
        #endif
        #if FF_HAVE_X11
            "x11\n"
        #endif
        #if FF_HAVE_DRM
            "drm\n"
        #endif
        #if FF_HAVE_GIO
            "gio\n"
        #endif
        #if FF_HAVE_DCONF
            "dconf\n"
        #endif
        #if FF_HAVE_DBUS
            "dbus\n"
        #endif
        #if FF_HAVE_IMAGEMAGICK7
            "imagemagick7\n"
        #endif
        #if FF_HAVE_IMAGEMAGICK6
            "imagemagick6\n"
        #endif
        #if FF_HAVE_CHAFA
            "chafa\n"
        #endif
        #if FF_HAVE_ZLIB
            "zlib\n"
        #endif
        #if FF_HAVE_XFCONF
            "xfconf\n"
        #endif
        #if FF_HAVE_SQLITE3
            "sqlite3\n"
        #endif
        #if FF_HAVE_RPM
            "rpm\n"
        #endif
        #if FF_HAVE_EGL
            "egl\n"
        #endif
        #if FF_HAVE_GLX
            "glx\n"
        #endif
        #if FF_HAVE_OSMESA
            "osmesa\n"
        #endif
        #if FF_HAVE_OPENCL
            "opencl\n"
        #endif
        #if FF_HAVE_FREETYPE
            "freetype\n"
        #endif
        #if FF_HAVE_PULSE
            "libpulse\n"
        #endif
        #if FF_HAVE_DDCUTIL
            "libddcutil\n"
        #endif
        #if FF_HAVE_ELF || __sun || __FreeBSD__
            "libelf\n"
        #endif
        #if FF_HAVE_LIBZFS
            "libzfs\n"
        #endif
        #if FF_HAVE_DIRECTX_HEADERS
            "Directx Headers\n"
        #endif
        #if FF_USE_SYSTEM_YYJSON
            "System yyjson\n"
        #endif
        #if FF_HAVE_LINUX_VIDEODEV2
            "linux/videodev2\n"
        #endif
        #if FF_HAVE_LINUX_WIRELESS
            "linux/wireless\n"
        #endif
        ""
    , stdout);
}
