#include "fastfetch.h"
#include "common/parsing.h"
#include "common/thread.h"
#include "detection/displayserver/displayserver.h"
#include "util/textModifier.h"
#include "logo/logo.h"

#include <stdlib.h>
#include <unistd.h>
#ifdef _WIN32
    #include <windows.h>
    #include <locale.h>
    #include "util/windows/unicode.h"
#else
    #include <signal.h>
#endif

#include "modules/modules.h"

FFinstance instance; // Global singleton

static void initState(FFstate* state)
{
    state->logoWidth = 0;
    state->logoHeight = 0;
    state->keysHeight = 0;
    state->titleLength = 0;

    ffPlatformInit(&state->platform);
    state->configDoc = NULL;
}

static void defaultConfig(void)
{
    ffInitLogoOptions(&instance.config.logo);

    ffStrbufInit(&instance.config.colorKeys);
    ffStrbufInit(&instance.config.colorTitle);
    ffStrbufInitS(&instance.config.keyValueSeparator, ": ");
    instance.config.processingTimeout = 1000;

    #if defined(__linux__) || defined(__FreeBSD__)
    ffStrbufInit(&instance.config.playerName);
    ffStrbufInit(&instance.config.osFile);
    #elif defined(_WIN32)
    instance.config.wmiTimeout = 5000;
    #endif

    instance.config.showErrors = false;
    instance.config.recache = false;
    instance.config.allowSlowOperations = false;
    instance.config.pipe = !isatty(STDOUT_FILENO);

    #ifdef NDEBUG
    instance.config.disableLinewrap = !instance.config.pipe;
    instance.config.hideCursor = !instance.config.pipe;
    #else
    instance.config.disableLinewrap = false;
    instance.config.hideCursor = false;
    #endif

    instance.config.escapeBedrock = true;
    instance.config.binaryPrefixType = FF_BINARY_PREFIX_TYPE_IEC;
    instance.config.sizeNdigits = 2;
    instance.config.sizeMaxPrefix = UINT8_MAX;
    instance.config.multithreading = true;
    instance.config.stat = false;
    instance.config.noBuffer = false;

    ffInitTitleOptions(&instance.config.title);
    ffInitOSOptions(&instance.config.os);
    ffInitHostOptions(&instance.config.host);
    ffInitBiosOptions(&instance.config.bios);
    ffInitBoardOptions(&instance.config.board);
    ffInitBrightnessOptions(&instance.config.brightness);
    ffInitChassisOptions(&instance.config.chassis);
    ffInitCommandOptions(&instance.config.command);
    ffInitCustomOptions(&instance.config.custom);
    ffInitKernelOptions(&instance.config.kernel);
    ffInitUptimeOptions(&instance.config.uptime);
    ffInitProcessesOptions(&instance.config.processes);
    ffInitPackagesOptions(&instance.config.packages);
    ffInitShellOptions(&instance.config.shell);
    ffInitDisplayOptions(&instance.config.display);
    ffInitDEOptions(&instance.config.de);
    ffInitWMOptions(&instance.config.wm);
    ffInitWMThemeOptions(&instance.config.wmTheme);
    ffInitThemeOptions(&instance.config.theme);
    ffInitIconsOptions(&instance.config.icons);
    ffInitFontOptions(&instance.config.font);
    ffInitCursorOptions(&instance.config.cursor);
    ffInitTerminalOptions(&instance.config.terminal);
    ffInitTerminalFontOptions(&instance.config.terminalFont);
    ffInitCPUOptions(&instance.config.cpu);
    ffInitCPUUsageOptions(&instance.config.cpuUsage);
    ffInitGPUOptions(&instance.config.gpu);
    ffInitMemoryOptions(&instance.config.memory);
    ffInitSwapOptions(&instance.config.swap);
    ffInitDiskOptions(&instance.config.disk);
    ffInitBatteryOptions(&instance.config.battery);
    ffInitPowerAdapterOptions(&instance.config.powerAdapter);
    ffInitLMOptions(&instance.config.lm);
    ffInitLocaleOptions(&instance.config.locale);
    ffInitLocalIpOptions(&instance.config.localIP);
    ffInitPublicIpOptions(&instance.config.publicIP);
    ffInitWeatherOptions(&instance.config.weather);
    ffInitWifiOptions(&instance.config.wifi);
    ffInitPlayerOptions(&instance.config.player);
    ffInitMediaOptions(&instance.config.media);
    ffInitDateTimeOptions(&instance.config.dateTime);
    ffInitVulkanOptions(&instance.config.vulkan);
    ffInitWallpaperOptions(&instance.config.wallpaper);
    ffInitOpenGLOptions(&instance.config.openGL);
    ffInitOpenCLOptions(&instance.config.openCL);
    ffInitUsersOptions(&instance.config.users);
    ffInitBluetoothOptions(&instance.config.bluetooth);
    ffInitSoundOptions(&instance.config.sound);
    ffInitSeparatorOptions(&instance.config.separator);
    ffInitGamepadOptions(&instance.config.gamepad);
    ffInitColorsOptions(&instance.config.colors);

    ffStrbufInit(&instance.config.libPCI);
    ffStrbufInit(&instance.config.libVulkan);
    ffStrbufInit(&instance.config.libWayland);
    ffStrbufInit(&instance.config.libXcbRandr);
    ffStrbufInit(&instance.config.libXcb);
    ffStrbufInit(&instance.config.libXrandr);
    ffStrbufInit(&instance.config.libX11);
    ffStrbufInit(&instance.config.libGIO);
    ffStrbufInit(&instance.config.libDConf);
    ffStrbufInit(&instance.config.libDBus);
    ffStrbufInit(&instance.config.libXFConf);
    ffStrbufInit(&instance.config.libSQLite3);
    ffStrbufInit(&instance.config.librpm);
    ffStrbufInit(&instance.config.libImageMagick);
    ffStrbufInit(&instance.config.libZ);
    ffStrbufInit(&instance.config.libChafa);
    ffStrbufInit(&instance.config.libEGL);
    ffStrbufInit(&instance.config.libGLX);
    ffStrbufInit(&instance.config.libOSMesa);
    ffStrbufInit(&instance.config.libOpenCL);
    ffStrbufInit(&instance.config.libfreetype);
    ffStrbufInit(&instance.config.libPulse);
    ffStrbufInit(&instance.config.libnm);
    ffStrbufInit(&instance.config.libDdcutil);

    instance.config.percentType = 1;
}

void ffInitInstance(void)
{
    #ifdef WIN32
        //https://learn.microsoft.com/en-us/cpp/c-runtime-library/reference/setlocale-wsetlocale?source=recommendations&view=msvc-170#utf-8-support
        setlocale(LC_ALL, ".UTF8");
    #endif

    initState(&instance.state);
    defaultConfig();
}

#if defined(FF_HAVE_THREADS) && !(defined(__APPLE__) || defined(_WIN32) || defined(__ANDROID__))

#include "detection/gtk_qt/gtk_qt.h"

#define FF_START_DETECTION_THREADS

FF_THREAD_ENTRY_DECL_WRAPPER_NOPARAM(ffConnectDisplayServer)
FF_THREAD_ENTRY_DECL_WRAPPER_NOPARAM(ffDetectQt)
FF_THREAD_ENTRY_DECL_WRAPPER_NOPARAM(ffDetectGTK2)
FF_THREAD_ENTRY_DECL_WRAPPER_NOPARAM(ffDetectGTK3)
FF_THREAD_ENTRY_DECL_WRAPPER_NOPARAM(ffDetectGTK4)

void startDetectionThreads(void)
{
    ffThreadDetach(ffThreadCreate(ffConnectDisplayServerThreadMain, NULL));
    ffThreadDetach(ffThreadCreate(ffDetectQtThreadMain, NULL));
    ffThreadDetach(ffThreadCreate(ffDetectGTK2ThreadMain, NULL));
    ffThreadDetach(ffThreadCreate(ffDetectGTK3ThreadMain, NULL));
    ffThreadDetach(ffThreadCreate(ffDetectGTK4ThreadMain, NULL));
}

#endif //FF_HAVE_THREADS

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
        if(instance.config.multithreading)
            startDetectionThreads();
    #endif

    ffDisableLinewrap = instance.config.disableLinewrap && !instance.config.pipe;
    ffHideCursor = instance.config.hideCursor && !instance.config.pipe;

    #ifdef _WIN32
    if (!instance.config.noBuffer) setvbuf(stdout, NULL, _IOFBF, 4096);
    SetConsoleCtrlHandler(consoleHandler, TRUE);
    HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD mode = 0;
    GetConsoleMode(hStdout, &mode);
    SetConsoleMode(hStdout, mode | ENABLE_PROCESSED_OUTPUT | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
    SetConsoleOutputCP(CP_UTF8);
    #else
    if (instance.config.noBuffer) setvbuf(stdout, NULL, _IONBF, 0);
    struct sigaction action = { .sa_handler = exitSignalHandler };
    sigaction(SIGINT, &action, NULL);
    sigaction(SIGTERM, &action, NULL);
    sigaction(SIGQUIT, &action, NULL);
    #endif

    //reset everything to default before we start printing
    if(!instance.config.pipe)
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
    ffDestroyLogoOptions(&instance.config.logo);

    ffStrbufDestroy(&instance.config.colorKeys);
    ffStrbufDestroy(&instance.config.colorTitle);
    ffStrbufDestroy(&instance.config.keyValueSeparator);

    #if defined(__linux__) || defined(__FreeBSD__)
    ffStrbufDestroy(&instance.config.playerName);
    ffStrbufDestroy(&instance.config.osFile);
    #endif

    ffDestroyTitleOptions(&instance.config.title);
    ffDestroyOSOptions(&instance.config.os);
    ffDestroyHostOptions(&instance.config.host);
    ffDestroyBiosOptions(&instance.config.bios);
    ffDestroyBoardOptions(&instance.config.board);
    ffDestroyBrightnessOptions(&instance.config.brightness);
    ffDestroyChassisOptions(&instance.config.chassis);
    ffDestroyCommandOptions(&instance.config.command);
    ffDestroyCustomOptions(&instance.config.custom);
    ffDestroyKernelOptions(&instance.config.kernel);
    ffDestroyUptimeOptions(&instance.config.uptime);
    ffDestroyProcessesOptions(&instance.config.processes);
    ffDestroyPackagesOptions(&instance.config.packages);
    ffDestroyShellOptions(&instance.config.shell);
    ffDestroyDisplayOptions(&instance.config.display);
    ffDestroyDEOptions(&instance.config.de);
    ffDestroyWMOptions(&instance.config.wm);
    ffDestroyWMThemeOptions(&instance.config.wmTheme);
    ffDestroyThemeOptions(&instance.config.theme);
    ffDestroyIconsOptions(&instance.config.icons);
    ffDestroyFontOptions(&instance.config.font);
    ffDestroyCursorOptions(&instance.config.cursor);
    ffDestroyTerminalOptions(&instance.config.terminal);
    ffDestroyTerminalFontOptions(&instance.config.terminalFont);
    ffDestroyCPUOptions(&instance.config.cpu);
    ffDestroyCPUUsageOptions(&instance.config.cpuUsage);
    ffDestroyGPUOptions(&instance.config.gpu);
    ffDestroyMemoryOptions(&instance.config.memory);
    ffDestroySwapOptions(&instance.config.swap);
    ffDestroyDiskOptions(&instance.config.disk);
    ffDestroyBatteryOptions(&instance.config.battery);
    ffDestroyPowerAdapterOptions(&instance.config.powerAdapter);
    ffDestroyLMOptions(&instance.config.lm);
    ffDestroyLocaleOptions(&instance.config.locale);
    ffDestroyLocalIpOptions(&instance.config.localIP);
    ffDestroyPublicIpOptions(&instance.config.publicIP);
    ffDestroyWallpaperOptions(&instance.config.wallpaper);
    ffDestroyWeatherOptions(&instance.config.weather);
    ffDestroyWifiOptions(&instance.config.wifi);
    ffDestroyPlayerOptions(&instance.config.player);
    ffDestroyMediaOptions(&instance.config.media);
    ffDestroyDateTimeOptions(&instance.config.dateTime);
    ffDestroyVulkanOptions(&instance.config.vulkan);
    ffDestroyOpenGLOptions(&instance.config.openGL);
    ffDestroyOpenCLOptions(&instance.config.openCL);
    ffDestroyUsersOptions(&instance.config.users);
    ffDestroyBluetoothOptions(&instance.config.bluetooth);
    ffDestroySeparatorOptions(&instance.config.separator);
    ffDestroySoundOptions(&instance.config.sound);
    ffDestroyGamepadOptions(&instance.config.gamepad);
    ffDestroyColorsOptions(&instance.config.colors);

    ffStrbufDestroy(&instance.config.libPCI);
    ffStrbufDestroy(&instance.config.libVulkan);
    ffStrbufDestroy(&instance.config.libWayland);
    ffStrbufDestroy(&instance.config.libXcbRandr);
    ffStrbufDestroy(&instance.config.libXcb);
    ffStrbufDestroy(&instance.config.libXrandr);
    ffStrbufDestroy(&instance.config.libX11);
    ffStrbufDestroy(&instance.config.libGIO);
    ffStrbufDestroy(&instance.config.libDConf);
    ffStrbufDestroy(&instance.config.libDBus);
    ffStrbufDestroy(&instance.config.libXFConf);
    ffStrbufDestroy(&instance.config.libSQLite3);
    ffStrbufDestroy(&instance.config.librpm);
    ffStrbufDestroy(&instance.config.libImageMagick);
    ffStrbufDestroy(&instance.config.libZ);
    ffStrbufDestroy(&instance.config.libChafa);
    ffStrbufDestroy(&instance.config.libEGL);
    ffStrbufDestroy(&instance.config.libGLX);
    ffStrbufDestroy(&instance.config.libOSMesa);
    ffStrbufDestroy(&instance.config.libOpenCL);
    ffStrbufDestroy(&instance.config.libfreetype);
    ffStrbufDestroy(&instance.config.libPulse);
    ffStrbufDestroy(&instance.config.libnm);
    ffStrbufDestroy(&instance.config.libDdcutil);
}

static void destroyState(void)
{
    ffPlatformDestroy(&instance.state.platform);
    yyjson_doc_free(instance.state.configDoc);
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
        #ifdef FF_HAVE_FREETYPE
            "freetype\n"
        #endif
        #ifdef FF_HAVE_PULSE
            "libpulse\n"
        #endif
        #ifdef FF_HAVE_LIBNM
            "libnm\n"
        #endif
        #ifdef FF_HAVE_DDCUTIL
            "libddcutil\n"
        #endif
        ""
    , stdout);
}
