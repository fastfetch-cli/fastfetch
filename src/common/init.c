#include "fastfetch.h"
#include "common/parsing.h"
#include "common/thread.h"
#include "detection/displayserver/displayserver.h"
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

#include "modules/modules.h"

FFinstance instance; // Global singleton

static void initState(FFstate* state)
{
    state->logoWidth = 0;
    state->logoHeight = 0;
    state->keysHeight = 0;

    ffPlatformInit(&state->platform);
    state->configDoc = NULL;
    state->resultDoc = NULL;
}

static void defaultConfig(void)
{
    ffInitLogoOptions(&instance.config.logo);

    ffStrbufInit(&instance.config.colorKeys);
    ffStrbufInit(&instance.config.colorTitle);
    instance.config.brightColor = true;
    ffStrbufInitStatic(&instance.config.keyValueSeparator, ": ");
    instance.config.processingTimeout = 1000;

    #if defined(__linux__) || defined(__FreeBSD__)
    ffStrbufInit(&instance.config.playerName);
    ffStrbufInit(&instance.config.osFile);
    instance.config.dsForceDrm = false;
    #elif defined(_WIN32)
    instance.config.wmiTimeout = 5000;
    #endif

    instance.config.showErrors = false;
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
    instance.config.temperatureUnit = FF_TEMPERATURE_UNIT_CELSIUS;
    instance.config.multithreading = true;
    instance.config.stat = false;
    instance.config.noBuffer = false;
    instance.config.keyWidth = 0;

    ffStrbufInitStatic(&instance.config.barCharElapsed, "■");
    ffStrbufInitStatic(&instance.config.barCharTotal, "-");
    instance.config.barWidth = 10;
    instance.config.barBorder = true;
    instance.config.percentType = 1;
    instance.config.percentNdigits = 0;

    ffInitBatteryOptions(&instance.config.battery);
    ffInitBiosOptions(&instance.config.bios);
    ffInitBluetoothOptions(&instance.config.bluetooth);
    ffInitBoardOptions(&instance.config.board);
    ffInitBreakOptions(&instance.config.break_);
    ffInitBrightnessOptions(&instance.config.brightness);
    ffInitCPUOptions(&instance.config.cpu);
    ffInitCPUUsageOptions(&instance.config.cpuUsage);
    ffInitChassisOptions(&instance.config.chassis);
    ffInitColorsOptions(&instance.config.colors);
    ffInitCommandOptions(&instance.config.command);
    ffInitCursorOptions(&instance.config.cursor);
    ffInitCustomOptions(&instance.config.custom);
    ffInitDEOptions(&instance.config.de);
    ffInitDateTimeOptions(&instance.config.dateTime);
    ffInitDiskOptions(&instance.config.disk);
    ffInitDisplayOptions(&instance.config.display);
    ffInitFontOptions(&instance.config.font);
    ffInitGPUOptions(&instance.config.gpu);
    ffInitGamepadOptions(&instance.config.gamepad);
    ffInitHostOptions(&instance.config.host);
    ffInitIconsOptions(&instance.config.icons);
    ffInitKernelOptions(&instance.config.kernel);
    ffInitLMOptions(&instance.config.lm);
    ffInitLocalIpOptions(&instance.config.localIP);
    ffInitLocaleOptions(&instance.config.locale);
    ffInitMediaOptions(&instance.config.media);
    ffInitMemoryOptions(&instance.config.memory);
    ffInitMonitorOptions(&instance.config.monitor);
    ffInitOSOptions(&instance.config.os);
    ffInitOpenCLOptions(&instance.config.openCL);
    ffInitOpenGLOptions(&instance.config.openGL);
    ffInitPackagesOptions(&instance.config.packages);
    ffInitPlayerOptions(&instance.config.player);
    ffInitPowerAdapterOptions(&instance.config.powerAdapter);
    ffInitProcessesOptions(&instance.config.processes);
    ffInitPublicIpOptions(&instance.config.publicIP);
    ffInitSeparatorOptions(&instance.config.separator);
    ffInitShellOptions(&instance.config.shell);
    ffInitSoundOptions(&instance.config.sound);
    ffInitSwapOptions(&instance.config.swap);
    ffInitTerminalFontOptions(&instance.config.terminalFont);
    ffInitTerminalOptions(&instance.config.terminal);
    ffInitTerminalSizeOptions(&instance.config.terminalSize);
    ffInitThemeOptions(&instance.config.theme);
    ffInitTitleOptions(&instance.config.title);
    ffInitUptimeOptions(&instance.config.uptime);
    ffInitUsersOptions(&instance.config.users);
    ffInitVersionOptions(&instance.config.version);
    ffInitVulkanOptions(&instance.config.vulkan);
    ffInitWMOptions(&instance.config.wm);
    ffInitWMThemeOptions(&instance.config.wmTheme);
    ffInitWallpaperOptions(&instance.config.wallpaper);
    ffInitWeatherOptions(&instance.config.weather);
    ffInitWifiOptions(&instance.config.wifi);

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
}

void ffInitInstance(void)
{
    #ifdef WIN32
        //https://learn.microsoft.com/en-us/cpp/c-runtime-library/reference/setlocale-wsetlocale?source=recommendations&view=msvc-170#utf-8-support
        setlocale(LC_ALL, ".UTF8");
    #else
        // used for mbsrtowcs in Module `separator`
        setlocale(LC_ALL, "");
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

    ffDisableLinewrap = instance.config.disableLinewrap && !instance.config.pipe && !instance.state.resultDoc;
    ffHideCursor = instance.config.hideCursor && !instance.config.pipe && !instance.state.resultDoc;

    #ifdef _WIN32
    setvbuf(stdout, NULL, _IOFBF, instance.config.noBuffer ? 0 : 4096);
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
    if(!instance.config.pipe && !instance.state.resultDoc)
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
    ffStrbufDestroy(&instance.config.barCharElapsed);
    ffStrbufDestroy(&instance.config.barCharTotal);

    #if defined(__linux__) || defined(__FreeBSD__)
    ffStrbufDestroy(&instance.config.playerName);
    ffStrbufDestroy(&instance.config.osFile);
    #endif

    ffDestroyBatteryOptions(&instance.config.battery);
    ffDestroyBiosOptions(&instance.config.bios);
    ffDestroyBluetoothOptions(&instance.config.bluetooth);
    ffDestroyBoardOptions(&instance.config.board);
    ffDestroyBreakOptions(&instance.config.break_);
    ffDestroyBrightnessOptions(&instance.config.brightness);
    ffDestroyCPUOptions(&instance.config.cpu);
    ffDestroyCPUUsageOptions(&instance.config.cpuUsage);
    ffDestroyChassisOptions(&instance.config.chassis);
    ffDestroyColorsOptions(&instance.config.colors);
    ffDestroyCommandOptions(&instance.config.command);
    ffDestroyCursorOptions(&instance.config.cursor);
    ffDestroyCustomOptions(&instance.config.custom);
    ffDestroyDEOptions(&instance.config.de);
    ffDestroyDateTimeOptions(&instance.config.dateTime);
    ffDestroyDiskOptions(&instance.config.disk);
    ffDestroyDisplayOptions(&instance.config.display);
    ffDestroyFontOptions(&instance.config.font);
    ffDestroyGPUOptions(&instance.config.gpu);
    ffDestroyGamepadOptions(&instance.config.gamepad);
    ffDestroyHostOptions(&instance.config.host);
    ffDestroyIconsOptions(&instance.config.icons);
    ffDestroyKernelOptions(&instance.config.kernel);
    ffDestroyLMOptions(&instance.config.lm);
    ffDestroyLocalIpOptions(&instance.config.localIP);
    ffDestroyLocaleOptions(&instance.config.locale);
    ffDestroyMediaOptions(&instance.config.media);
    ffDestroyMemoryOptions(&instance.config.memory);
    ffDestroyMonitorOptions(&instance.config.monitor);
    ffDestroyOSOptions(&instance.config.os);
    ffDestroyOpenCLOptions(&instance.config.openCL);
    ffDestroyOpenGLOptions(&instance.config.openGL);
    ffDestroyPackagesOptions(&instance.config.packages);
    ffDestroyPlayerOptions(&instance.config.player);
    ffDestroyPowerAdapterOptions(&instance.config.powerAdapter);
    ffDestroyProcessesOptions(&instance.config.processes);
    ffDestroyPublicIpOptions(&instance.config.publicIP);
    ffDestroySeparatorOptions(&instance.config.separator);
    ffDestroyShellOptions(&instance.config.shell);
    ffDestroySoundOptions(&instance.config.sound);
    ffDestroySwapOptions(&instance.config.swap);
    ffDestroyTerminalFontOptions(&instance.config.terminalFont);
    ffDestroyTerminalOptions(&instance.config.terminal);
    ffDestroyTerminalSizeOptions(&instance.config.terminalSize);
    ffDestroyThemeOptions(&instance.config.theme);
    ffDestroyTitleOptions(&instance.config.title);
    ffDestroyUptimeOptions(&instance.config.uptime);
    ffDestroyUsersOptions(&instance.config.users);
    ffDestroyVersionOptions(&instance.config.version);
    ffDestroyVulkanOptions(&instance.config.vulkan);
    ffDestroyWMOptions(&instance.config.wm);
    ffDestroyWMThemeOptions(&instance.config.wmTheme);
    ffDestroyWallpaperOptions(&instance.config.wallpaper);
    ffDestroyWeatherOptions(&instance.config.weather);
    ffDestroyWifiOptions(&instance.config.wifi);

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
    yyjson_mut_doc_free(instance.state.resultDoc);
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
