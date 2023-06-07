#include "fastfetch.h"
#include "common/parsing.h"
#include "common/thread.h"
#include "detection/displayserver/displayserver.h"
#include "util/textModifier.h"
#include "logo/logo.h"

#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#ifdef _WIN32
    #include <windows.h>
    #include <locale.h>
    #include "util/windows/unicode.h"
#else
    #include <signal.h>
#endif

#include "modules/modules.h"

static void initState(FFstate* state)
{
    state->logoWidth = 0;
    state->logoHeight = 0;
    state->keysHeight = 0;
    state->titleLength = 0;

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
    ffInitLogoOptions(&instance->config.logo);

    ffStrbufInit(&instance->config.colorKeys);
    ffStrbufInit(&instance->config.colorTitle);

    ffStrbufInit(&instance->config.keyValueSeparator);
    ffStrbufAppendS(&instance->config.keyValueSeparator, ": ");

    instance->config.showErrors = false;
    instance->config.recache = false;
    instance->config.allowSlowOperations = false;
    instance->config.disableLinewrap = true;
    instance->config.hideCursor = true;
    instance->config.escapeBedrock = true;
    instance->config.binaryPrefixType = FF_BINARY_PREFIX_TYPE_IEC;
    instance->config.pipe = false;
    instance->config.multithreading = true;
    instance->config.stat = false;

    ffInitTitleOptions(&instance->config.title);
    ffInitOSOptions(&instance->config.os);
    ffInitHostOptions(&instance->config.host);
    ffInitBiosOptions(&instance->config.bios);
    ffInitBoardOptions(&instance->config.board);
    ffInitBrightnessOptions(&instance->config.brightness);
    initModuleArg(&instance->config.chassis);
    ffInitCommandOptions(&instance->config.command);
    ffInitCustomOptions(&instance->config.custom);
    ffInitKernelOptions(&instance->config.kernel);
    ffInitUptimeOptions(&instance->config.uptime);
    initModuleArg(&instance->config.processes);
    ffInitPackagesOptions(&instance->config.packages);
    ffInitShellOptions(&instance->config.shell);
    ffInitDisplayOptions(&instance->config.display);
    ffInitDEOptions(&instance->config.de);
    ffInitWMOptions(&instance->config.wm);
    ffInitWMThemeOptions(&instance->config.wmTheme);
    initModuleArg(&instance->config.theme);
    ffInitIconsOptions(&instance->config.icons);
    ffInitFontOptions(&instance->config.font);
    ffInitCursorOptions(&instance->config.cursor);
    ffInitTerminalOptions(&instance->config.terminal);
    ffInitTerminalFontOptions(&instance->config.terminalFont);
    ffInitCPUOptions(&instance->config.cpu);
    ffInitCPUUsageOptions(&instance->config.cpuUsage);
    ffInitGPUOptions(&instance->config.gpu);
    ffInitMemoryOptions(&instance->config.memory);
    ffInitSwapOptions(&instance->config.swap);
    ffInitDiskOptions(&instance->config.disk);
    ffInitBatteryOptions(&instance->config.battery);
    ffInitPowerAdapterOptions(&instance->config.powerAdapter);
    ffInitLocaleOptions(&instance->config.locale);
    ffInitLocalIpOptions(&instance->config.localIP);
    initModuleArg(&instance->config.publicIP);
    initModuleArg(&instance->config.weather);
    ffInitWifiOptions(&instance->config.wifi);
    initModuleArg(&instance->config.player);
    initModuleArg(&instance->config.media);
    ffInitDateTimeOptions(&instance->config.dateTime);
    ffInitVulkanOptions(&instance->config.vulkan);
    ffInitWallpaperOptions(&instance->config.wallpaper);
    ffInitOpenGLOptions(&instance->config.openGL);
    ffInitOpenCLOptions(&instance->config.openCL);
    initModuleArg(&instance->config.users);
    ffInitBluetoothOptions(&instance->config.bluetooth);
    ffInitSoundOptions(&instance->config.sound);
    ffInitSeparatorOptions(&instance->config.separator);
    ffInitGamepadOptions(&instance->config.gamepad);

    ffStrbufInit(&instance->config.libPCI);
    ffStrbufInit(&instance->config.libVulkan);
    ffStrbufInit(&instance->config.libWayland);
    ffStrbufInit(&instance->config.libXcbRandr);
    ffStrbufInit(&instance->config.libXcb);
    ffStrbufInit(&instance->config.libXrandr);
    ffStrbufInit(&instance->config.libX11);
    ffStrbufInit(&instance->config.libGIO);
    ffStrbufInit(&instance->config.libDConf);
    ffStrbufInit(&instance->config.libDBus);
    ffStrbufInit(&instance->config.libXFConf);
    ffStrbufInit(&instance->config.libSQLite3);
    ffStrbufInit(&instance->config.librpm);
    ffStrbufInit(&instance->config.libImageMagick);
    ffStrbufInit(&instance->config.libZ);
    ffStrbufInit(&instance->config.libChafa);
    ffStrbufInit(&instance->config.libEGL);
    ffStrbufInit(&instance->config.libGLX);
    ffStrbufInit(&instance->config.libOSMesa);
    ffStrbufInit(&instance->config.libOpenCL);
    ffStrbufInit(&instance->config.libJSONC);
    ffStrbufInit(&instance->config.libfreetype);
    ffStrbufInit(&instance->config.libPulse);
    ffStrbufInit(&instance->config.libwlanapi);
    ffStrbufInit(&instance->config.libnm);

    instance->config.publicIpTimeout = 0;
    ffStrbufInit(&instance->config.publicIpUrl);

    instance->config.weatherTimeout = 0;
    ffStrbufInitS(&instance->config.weatherOutputFormat, "%t+-+%C+(%l)");

    ffStrbufInit(&instance->config.playerName);

    instance->config.percentType = 1;
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

#if defined(FF_HAVE_THREADS) && !(defined(__APPLE__) || defined(_WIN32) || defined(__ANDROID__))

#include "detection/gtk_qt/gtk_qt.h"

#define FF_START_DETECTION_THREADS

FF_THREAD_ENTRY_DECL_WRAPPER(ffConnectDisplayServer, FFinstance*)
FF_THREAD_ENTRY_DECL_WRAPPER(ffDetectQt, FFinstance*)
FF_THREAD_ENTRY_DECL_WRAPPER(ffDetectGTK2, FFinstance*)
FF_THREAD_ENTRY_DECL_WRAPPER(ffDetectGTK3, FFinstance*)
FF_THREAD_ENTRY_DECL_WRAPPER(ffDetectGTK4, FFinstance*)

void startDetectionThreads(FFinstance* instance)
{
    ffThreadDetach(ffThreadCreate(ffConnectDisplayServerThreadMain, instance));
    ffThreadDetach(ffThreadCreate(ffDetectQtThreadMain, instance));
    ffThreadDetach(ffThreadCreate(ffDetectGTK2ThreadMain, instance));
    ffThreadDetach(ffThreadCreate(ffDetectGTK3ThreadMain, instance));
    ffThreadDetach(ffThreadCreate(ffDetectGTK4ThreadMain, instance));
}

#endif //FF_HAVE_THREADS

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
    #ifdef FF_START_DETECTION_THREADS
        if(instance->config.multithreading)
            startDetectionThreads(instance);
    #endif

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
    ffDestroyLogoOptions(&instance->config.logo);

    ffStrbufDestroy(&instance->config.colorKeys);
    ffStrbufDestroy(&instance->config.colorTitle);
    ffStrbufDestroy(&instance->config.keyValueSeparator);

    ffDestroyTitleOptions(&instance->config.title);
    ffDestroyOSOptions(&instance->config.os);
    ffDestroyHostOptions(&instance->config.host);
    ffDestroyBiosOptions(&instance->config.bios);
    ffDestroyBoardOptions(&instance->config.board);
    ffDestroyBrightnessOptions(&instance->config.brightness);
    destroyModuleArg(&instance->config.chassis);
    ffDestroyCommandOptions(&instance->config.command);
    ffDestroyCustomOptions(&instance->config.custom);
    ffDestroyKernelOptions(&instance->config.kernel);
    ffDestroyUptimeOptions(&instance->config.uptime);
    destroyModuleArg(&instance->config.processes);
    ffDestroyPackagesOptions(&instance->config.packages);
    ffDestroyShellOptions(&instance->config.shell);
    ffDestroyDisplayOptions(&instance->config.display);
    ffDestroyDEOptions(&instance->config.de);
    ffDestroyWMOptions(&instance->config.wm);
    ffDestroyWMThemeOptions(&instance->config.wmTheme);
    destroyModuleArg(&instance->config.theme);
    ffDestroyIconsOptions(&instance->config.icons);
    ffDestroyFontOptions(&instance->config.font);
    ffDestroyCursorOptions(&instance->config.cursor);
    ffDestroyTerminalOptions(&instance->config.terminal);
    ffDestroyTerminalFontOptions(&instance->config.terminalFont);
    ffDestroyCPUOptions(&instance->config.cpu);
    ffDestroyCPUUsageOptions(&instance->config.cpuUsage);
    ffDestroyGPUOptions(&instance->config.gpu);
    ffDestroyMemoryOptions(&instance->config.memory);
    ffDestroySwapOptions(&instance->config.swap);
    ffDestroyDiskOptions(&instance->config.disk);
    ffDestroyBatteryOptions(&instance->config.battery);
    ffDestroyPowerAdapterOptions(&instance->config.powerAdapter);
    ffDestroyLocaleOptions(&instance->config.locale);
    ffDestroyLocalIpOptions(&instance->config.localIP);
    destroyModuleArg(&instance->config.publicIP);
    ffDestroyWallpaperOptions(&instance->config.wallpaper);
    destroyModuleArg(&instance->config.weather);
    ffDestroyWifiOptions(&instance->config.wifi);
    destroyModuleArg(&instance->config.player);
    destroyModuleArg(&instance->config.media);
    ffDestroyDateTimeOptions(&instance->config.dateTime);
    ffDestroyVulkanOptions(&instance->config.vulkan);
    ffDestroyOpenGLOptions(&instance->config.openGL);
    ffDestroyOpenCLOptions(&instance->config.openCL);
    destroyModuleArg(&instance->config.users);
    ffDestroyBluetoothOptions(&instance->config.bluetooth);
    ffDestroySeparatorOptions(&instance->config.separator);
    ffDestroySoundOptions(&instance->config.sound);
    ffDestroyGamepadOptions(&instance->config.gamepad);

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
    ffStrbufDestroy(&instance->config.libJSONC);
    ffStrbufDestroy(&instance->config.libfreetype);
    ffStrbufDestroy(&instance->config.libPulse);
    ffStrbufDestroy(&instance->config.libwlanapi);
    ffStrbufDestroy(&instance->config.libnm);

    ffStrbufDestroy(&instance->config.publicIpUrl);
    ffStrbufDestroy(&instance->config.weatherOutputFormat);
    ffStrbufDestroy(&instance->config.playerName);
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
        #ifdef FF_HAVE_THREADS
            "threads\n"
        #endif
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
        #ifdef FF_HAVE_LIBJSONC
            "json-c\n"
        #endif
        #ifdef FF_HAVE_FREETYPE
            "freetype\n"
        #endif
        #ifdef FF_HAVE_PULSE
            "libpulse\n"
        #endif
        #ifdef FF_HAVE_LIBNM
            "libnm\n"
        #endif
        ""
    , stdout);
}
