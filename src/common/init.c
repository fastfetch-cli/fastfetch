#include "fastfetch.h"
#include "common/parsing.h"
#include "common/thread.h"
#include "detection/qt.h"
#include "detection/gtk.h"
#include "detection/displayserver/displayserver.h"
#include "util/textModifier.h"

#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#ifdef _WIN32
    #include <wincon.h>
    #include <locale.h>
    #include <shlobj.h>
    #include "util/windows/unicode.h"
#else
    #include <signal.h>
#endif

static void initState(FFstate* state)
{
    state->logoWidth = 0;
    state->logoHeight = 0;
    state->keysHeight = 0;

    ffPlatformInit(&state->platform);
}

static void initModuleArg(FFModuleArgs* args)
{
    ffStrbufInit(&args->key);
    ffStrbufInit(&args->outputFormat);
    ffStrbufInit(&args->errorFormat);
}

static void defaultConfig(FFinstance* instance)
{
    ffStrbufInitA(&instance->config.logo.source, 0);
    instance->config.logo.type = FF_LOGO_TYPE_AUTO;
    for(uint8_t i = 0; i < (uint8_t) FASTFETCH_LOGO_MAX_COLORS; ++i)
        ffStrbufInit(&instance->config.logo.colors[i]);
    instance->config.logo.width = 0;
    instance->config.logo.height = 0; //preserve aspect ratio
    instance->config.logo.paddingTop = 0;
    instance->config.logo.paddingLeft = 0;
    instance->config.logo.paddingRight = 4;
    instance->config.logo.printRemaining = true;
    instance->config.logo.preserveAspectRadio = false;

    instance->config.logo.chafaFgOnly = false;
    ffStrbufInitS(&instance->config.logo.chafaSymbols, "block+border+space-wide-inverted"); // Chafa default
    instance->config.logo.chafaCanvasMode = UINT32_MAX;
    instance->config.logo.chafaColorSpace = UINT32_MAX;
    instance->config.logo.chafaDitherMode = UINT32_MAX;

    ffStrbufInit(&instance->config.colorKeys);
    ffStrbufInit(&instance->config.colorTitle);

    ffStrbufInit(&instance->config.separator);
    ffStrbufAppendS(&instance->config.separator, ": ");

    instance->config.showErrors = false;
    instance->config.recache = false;
    instance->config.allowSlowOperations = false;
    instance->config.disableLinewrap = true;
    instance->config.hideCursor = true;
    instance->config.escapeBedrock = true;
    instance->config.binaryPrefixType = FF_BINARY_PREFIX_TYPE_IEC;
    instance->config.glType = FF_GL_TYPE_AUTO;
    instance->config.pipe = false;
    instance->config.multithreading = true;
    instance->config.stat = false;

    initModuleArg(&instance->config.os);
    initModuleArg(&instance->config.host);
    initModuleArg(&instance->config.bios);
    initModuleArg(&instance->config.board);
    initModuleArg(&instance->config.brightness);
    initModuleArg(&instance->config.chassis);
    initModuleArg(&instance->config.kernel);
    initModuleArg(&instance->config.uptime);
    initModuleArg(&instance->config.processes);
    initModuleArg(&instance->config.packages);
    initModuleArg(&instance->config.shell);
    initModuleArg(&instance->config.resolution);
    initModuleArg(&instance->config.de);
    initModuleArg(&instance->config.wm);
    initModuleArg(&instance->config.wmTheme);
    initModuleArg(&instance->config.theme);
    initModuleArg(&instance->config.icons);
    initModuleArg(&instance->config.font);
    initModuleArg(&instance->config.cursor);
    initModuleArg(&instance->config.terminal);
    initModuleArg(&instance->config.terminalFont);
    initModuleArg(&instance->config.cpu);
    initModuleArg(&instance->config.cpuUsage);
    initModuleArg(&instance->config.gpu);
    initModuleArg(&instance->config.memory);
    initModuleArg(&instance->config.swap);
    initModuleArg(&instance->config.disk);
    initModuleArg(&instance->config.battery);
    initModuleArg(&instance->config.powerAdapter);
    initModuleArg(&instance->config.locale);
    initModuleArg(&instance->config.localIP);
    initModuleArg(&instance->config.publicIP);
    initModuleArg(&instance->config.weather);
    initModuleArg(&instance->config.wifi);
    initModuleArg(&instance->config.player);
    initModuleArg(&instance->config.media);
    initModuleArg(&instance->config.dateTime);
    initModuleArg(&instance->config.date);
    initModuleArg(&instance->config.time);
    initModuleArg(&instance->config.vulkan);
    initModuleArg(&instance->config.openGL);
    initModuleArg(&instance->config.openCL);
    initModuleArg(&instance->config.users);

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
    ffStrbufInitA(&instance->config.libcJSON, 0);
    ffStrbufInitA(&instance->config.libfreetype, 0);
    ffStrbufInit(&instance->config.libwlanapi);

    instance->config.cpuTemp = false;
    instance->config.gpuTemp = false;
    instance->config.batteryTemp = false;

    instance->config.gpuHideIntegrated = false;
    instance->config.gpuHideDiscrete = false;

    instance->config.shellVersion = true;
    instance->config.terminalVersion = true;

    instance->config.titleFQDN = false;

    ffStrbufInitA(&instance->config.diskFolders, 0);
    instance->config.diskShowRemovable = true;
    instance->config.diskShowHidden = false;
    instance->config.diskShowUnknown = false;

    ffStrbufInitA(&instance->config.batteryDir, 0);

    ffStrbufInitA(&instance->config.separatorString, 0);

    instance->config.localIpShowIpV4 = true;
    instance->config.localIpShowIpV6 = false;
    instance->config.localIpShowLoop = false;
    ffStrbufInit(&instance->config.localIpNamePrefix);

    instance->config.publicIpTimeout = 0;
    ffStrbufInit(&instance->config.publicIpUrl);

    instance->config.weatherTimeout = 0;
    ffStrbufInitS(&instance->config.weatherOutputFormat, "%t+-+%C+(%l)");

    ffStrbufInitA(&instance->config.osFile, 0);

    ffStrbufInitA(&instance->config.playerName, 0);

    instance->config.percentType = 1;

    ffStrbufInitS(&instance->config.commandShell,
        #ifdef _WIN32
        "cmd"
        #elif defined(__FreeBSD__)
        "csh"
        #else
        "bash"
        #endif
    );
    ffListInit(&instance->config.commandKeys, sizeof(FFstrbuf));
    ffListInit(&instance->config.commandTexts, sizeof(FFstrbuf));
}

void ffInitInstance(FFinstance* instance)
{
    #ifdef WIN32
        //https://learn.microsoft.com/en-us/cpp/c-runtime-library/reference/setlocale-wsetlocale?source=recommendations&view=msvc-170#utf-8-support
        setlocale(LC_ALL, ".UTF8");
    #endif

    initState(&instance->state);
    defaultConfig(instance);
}

#ifdef FF_HAVE_THREADS

FF_THREAD_ENTRY_DECL_WRAPPER(ffConnectDisplayServer, FFinstance*)

#if !(defined(__APPLE__) || defined(_WIN32))

#define FF_DETECT_QT_GTK 1

FF_THREAD_ENTRY_DECL_WRAPPER(ffDetectQt, FFinstance*)
FF_THREAD_ENTRY_DECL_WRAPPER(ffDetectGTK2, FFinstance*)
FF_THREAD_ENTRY_DECL_WRAPPER(ffDetectGTK3, FFinstance*)
FF_THREAD_ENTRY_DECL_WRAPPER(ffDetectGTK4, FFinstance*)

#endif //!(defined(__APPLE__) || defined(_WIN32))

#endif //FF_HAVE_THREADS

void startDetectionThreads(FFinstance* instance)
{
    #ifdef FF_HAVE_THREADS
    ffThreadDetach(ffThreadCreate(ffConnectDisplayServerThreadMain, instance));

    #ifdef FF_DETECT_QT_GTK
    ffThreadDetach(ffThreadCreate(ffDetectQtThreadMain, instance));
    ffThreadDetach(ffThreadCreate(ffDetectGTK2ThreadMain, instance));
    ffThreadDetach(ffThreadCreate(ffDetectGTK3ThreadMain, instance));
    ffThreadDetach(ffThreadCreate(ffDetectGTK4ThreadMain, instance));
    #endif

    #else
    FF_UNUSED(instance);
    #endif
}

static volatile bool ffDisableLinewrap = true;
static volatile bool ffHideCursor = true;

static void resetConsole()
{
    if(ffDisableLinewrap)
        fputs("\033[?7h", stdout);

    if(ffHideCursor)
        fputs("\033[?25h", stdout);

    #if defined(_WIN32) && defined(FF_ENABLE_BUFFER)
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

void ffStart(FFinstance* instance)
{
    if(instance->config.multithreading)
        startDetectionThreads(instance);

    ffDisableLinewrap = instance->config.disableLinewrap && !instance->config.pipe;
    ffHideCursor = instance->config.hideCursor && !instance->config.pipe;

    #ifdef _WIN32
    #ifdef FF_ENABLE_BUFFER
        setvbuf(stdout, NULL, _IOFBF, 4096);
    #endif
    SetConsoleCtrlHandler(consoleHandler, TRUE);
    HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD mode = 0;
    GetConsoleMode(hStdout, &mode);
    SetConsoleMode(hStdout, mode | ENABLE_PROCESSED_OUTPUT | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
    SetConsoleOutputCP(CP_UTF8);
    #else
    #ifndef FF_ENABLE_BUFFER
        setvbuf(stdout, NULL, _IONBF, 0);
    #endif
    struct sigaction action = { .sa_handler = exitSignalHandler };
    sigaction(SIGINT, &action, NULL);
    sigaction(SIGTERM, &action, NULL);
    sigaction(SIGQUIT, &action, NULL);
    #endif

    //reset everything to default before we start printing
    if(!instance->config.pipe)
        fputs(FASTFETCH_TEXT_MODIFIER_RESET, stdout);

    if(ffHideCursor)
        fputs("\033[?25l", stdout);

    if(ffDisableLinewrap)
        fputs("\033[?7l", stdout);

    ffLogoPrint(instance);
}

void ffFinish(FFinstance* instance)
{
    if(instance->config.logo.printRemaining)
        ffLogoPrintRemaining(instance);

    resetConsole();
}

static void destroyModuleArg(FFModuleArgs* args)
{
    ffStrbufDestroy(&args->key);
    ffStrbufDestroy(&args->outputFormat);
    ffStrbufDestroy(&args->errorFormat);
}

static void destroyConfig(FFinstance* instance)
{
    ffStrbufDestroy(&instance->config.logo.source);
    ffStrbufDestroy(&instance->config.logo.chafaSymbols);
    for(uint8_t i = 0; i < (uint8_t) FASTFETCH_LOGO_MAX_COLORS; ++i)
        ffStrbufDestroy(&instance->config.logo.colors[i]);
    ffStrbufDestroy(&instance->config.colorKeys);
    ffStrbufDestroy(&instance->config.colorTitle);
    ffStrbufDestroy(&instance->config.separator);

    destroyModuleArg(&instance->config.os);
    destroyModuleArg(&instance->config.host);
    destroyModuleArg(&instance->config.bios);
    destroyModuleArg(&instance->config.board);
    destroyModuleArg(&instance->config.kernel);
    destroyModuleArg(&instance->config.uptime);
    destroyModuleArg(&instance->config.processes);
    destroyModuleArg(&instance->config.packages);
    destroyModuleArg(&instance->config.shell);
    destroyModuleArg(&instance->config.resolution);
    destroyModuleArg(&instance->config.de);
    destroyModuleArg(&instance->config.wm);
    destroyModuleArg(&instance->config.wmTheme);
    destroyModuleArg(&instance->config.theme);
    destroyModuleArg(&instance->config.icons);
    destroyModuleArg(&instance->config.font);
    destroyModuleArg(&instance->config.cursor);
    destroyModuleArg(&instance->config.terminal);
    destroyModuleArg(&instance->config.terminalFont);
    destroyModuleArg(&instance->config.cpu);
    destroyModuleArg(&instance->config.cpuUsage);
    destroyModuleArg(&instance->config.gpu);
    destroyModuleArg(&instance->config.memory);
    destroyModuleArg(&instance->config.swap);
    destroyModuleArg(&instance->config.disk);
    destroyModuleArg(&instance->config.battery);
    destroyModuleArg(&instance->config.powerAdapter);
    destroyModuleArg(&instance->config.locale);
    destroyModuleArg(&instance->config.localIP);
    destroyModuleArg(&instance->config.publicIP);
    destroyModuleArg(&instance->config.weather);
    destroyModuleArg(&instance->config.wifi);
    destroyModuleArg(&instance->config.player);
    destroyModuleArg(&instance->config.media);
    destroyModuleArg(&instance->config.dateTime);
    destroyModuleArg(&instance->config.date);
    destroyModuleArg(&instance->config.time);
    destroyModuleArg(&instance->config.vulkan);
    destroyModuleArg(&instance->config.openGL);
    destroyModuleArg(&instance->config.openCL);
    destroyModuleArg(&instance->config.users);

    ffStrbufDestroy(&instance->config.libPCI);
    ffStrbufDestroy(&instance->config.libVulkan);
    ffStrbufDestroy(&instance->config.libWayland);
    ffStrbufDestroy(&instance->config.libXcbRandr);
    ffStrbufDestroy(&instance->config.libXcb);
    ffStrbufDestroy(&instance->config.libXrandr);
    ffStrbufDestroy(&instance->config.libX11);
    ffStrbufDestroy(&instance->config.libGIO);
    ffStrbufDestroy(&instance->config.libDConf);
    ffStrbufDestroy(&instance->config.libDBus);
    ffStrbufDestroy(&instance->config.libXFConf);
    ffStrbufDestroy(&instance->config.libSQLite3);
    ffStrbufDestroy(&instance->config.librpm);
    ffStrbufDestroy(&instance->config.libImageMagick);
    ffStrbufDestroy(&instance->config.libZ);
    ffStrbufDestroy(&instance->config.libChafa);
    ffStrbufDestroy(&instance->config.libEGL);
    ffStrbufDestroy(&instance->config.libGLX);
    ffStrbufDestroy(&instance->config.libOSMesa);
    ffStrbufDestroy(&instance->config.libOpenCL);
    ffStrbufDestroy(&instance->config.libcJSON);
    ffStrbufDestroy(&instance->config.libfreetype);
    ffStrbufDestroy(&instance->config.libwlanapi);

    ffStrbufDestroy(&instance->config.diskFolders);
    ffStrbufDestroy(&instance->config.batteryDir);
    ffStrbufDestroy(&instance->config.separatorString);
    ffStrbufDestroy(&instance->config.localIpNamePrefix);
    ffStrbufDestroy(&instance->config.publicIpUrl);
    ffStrbufDestroy(&instance->config.weatherOutputFormat);
    ffStrbufDestroy(&instance->config.osFile);
    ffStrbufDestroy(&instance->config.playerName);

    ffStrbufDestroy(&instance->config.commandShell);
    FF_LIST_FOR_EACH(FFstrbuf, item, instance->config.commandKeys)
        ffStrbufDestroy(item);
    ffListDestroy(&instance->config.commandKeys);
    FF_LIST_FOR_EACH(FFstrbuf, item, instance->config.commandTexts)
        ffStrbufDestroy(item);
    ffListDestroy(&instance->config.commandTexts);
}

static void destroyState(FFinstance* instance)
{
    ffPlatformDestroy(&instance->state.platform);
}

void ffDestroyInstance(FFinstance* instance)
{
    destroyConfig(instance);
    destroyState(instance);
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
        #ifdef FF_HAVE_LIBCJSON
            "libcjson\n"
        #endif
        #ifdef FF_HAVE_FREETYPE
            "freetype"
        #endif
        ""
    , stdout);
}
