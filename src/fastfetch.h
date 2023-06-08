#pragma once

#ifndef FASTFETCH_INCLUDED
#define FASTFETCH_INCLUDED

#include "fastfetch_config.h"

#include <stdint.h>
#include <stdbool.h>

#include "util/FFstrbuf.h"
#include "util/FFlist.h"
#include "util/platform/FFPlatform.h"

static inline void ffUnused(int dummy, ...) { (void) dummy; }
#define FF_UNUSED(...) ffUnused(0, __VA_ARGS__);
#define FF_MAYBE_UNUSED __attribute__ ((__unused__))

#define FASTFETCH_LOGO_MAX_COLORS 9 //two digits would make parsing much more complicated (index 1 - 9)

typedef enum FFLogoType
{
    FF_LOGO_TYPE_AUTO,        //if something is given, first try builtin, then file. Otherwise detect logo
    FF_LOGO_TYPE_BUILTIN,     //builtin ascii art
    FF_LOGO_TYPE_FILE,        //text file, printed with color code replacement
    FF_LOGO_TYPE_FILE_RAW,    //text file, printed as is
    FF_LOGO_TYPE_DATA,        //text data, printed with color code replacement
    FF_LOGO_TYPE_DATA_RAW,    //text data, printed as is
    FF_LOGO_TYPE_IMAGE_SIXEL, //image file, printed as sixel codes.
    FF_LOGO_TYPE_IMAGE_KITTY, //image file, printed as kitty graphics protocol
    FF_LOGO_TYPE_IMAGE_ITERM, //image file, printed as iterm graphics protocol
    FF_LOGO_TYPE_IMAGE_CHAFA, //image file, printed as ascii art using libchafa
    FF_LOGO_TYPE_IMAGE_RAW,   //image file, printed as raw binary string
    FF_LOGO_TYPE_NONE,        //--logo none
} FFLogoType;

typedef enum FFSoundType
{
    FF_SOUND_TYPE_MAIN,
    FF_SOUND_TYPE_ACTIVE,
    FF_SOUND_TYPE_ALL,
} FFSoundType;

typedef enum FFDisplayCompactType
{
    FF_DISPLAY_COMPACT_TYPE_NONE = 0,
    FF_DISPLAY_COMPACT_TYPE_ORIGINAL_BIT = 1 << 0,
    FF_DISPLAY_COMPACT_TYPE_SCALED_BIT = 1 << 1,
} FFDisplayCompactType;

typedef enum FFLocalIpCompactType
{
    FF_LOCALIP_COMPACT_TYPE_NONE,
    FF_LOCALIP_COMPACT_TYPE_MULTILINE,
    FF_LOCALIP_COMPACT_TYPE_ONELINE,
} FFLocalIpCompactType;

typedef enum FFBinaryPrefixType
{
    FF_BINARY_PREFIX_TYPE_IEC,   // 1024 Bytes = 1 KiB, 1024 KiB = 1 MiB, ... (standard)
    FF_BINARY_PREFIX_TYPE_SI,    // 1000 Bytes = 1 KB, 1000 KB = 1 MB, ...
    FF_BINARY_PREFIX_TYPE_JEDEC, // 1024 Bytes = 1 kB, 1024 K = 1 MB, ...
} FFBinaryPrefixType;

typedef enum FFGLType
{
    FF_GL_TYPE_AUTO,
    FF_GL_TYPE_EGL,
    FF_GL_TYPE_GLX,
    FF_GL_TYPE_OSMESA
} FFGLType;

typedef struct FFModuleArgs
{
    FFstrbuf key;
    FFstrbuf outputFormat;
    FFstrbuf errorFormat;
} FFModuleArgs;

typedef enum FFLocalIpType
{
    FF_LOCALIP_TYPE_NONE,
    FF_LOCALIP_TYPE_LOOP_BIT = 1 << 0,
    FF_LOCALIP_TYPE_IPV4_BIT = 1 << 1,
    FF_LOCALIP_TYPE_IPV6_BIT = 1 << 2,
    FF_LOCALIP_TYPE_MAC_BIT  = 1 << 3,
    FF_LOCALIP_TYPE_TYPE_BIT = 1 << 4,

    FF_LOCALIP_TYPE_COMPACT_BIT = 1 << 10,
} FFLocalIpType;

typedef struct FFconfig
{
    struct
    {
        FFstrbuf source;
        FFLogoType type;
        FFstrbuf colors[FASTFETCH_LOGO_MAX_COLORS];
        uint32_t width;
        uint32_t height;
        uint32_t paddingTop;
        uint32_t paddingLeft;
        uint32_t paddingRight;
        bool printRemaining;
        bool preserveAspectRadio;

        bool chafaFgOnly;
        FFstrbuf chafaSymbols;
        uint32_t chafaCanvasMode;
        uint32_t chafaColorSpace;
        uint32_t chafaDitherMode;
    } logo;

    //If one of those is empty, ffLogoPrint will set them
    FFstrbuf colorKeys;
    FFstrbuf colorTitle;

    FFstrbuf separator;

    bool showErrors;
    bool recache;
    bool allowSlowOperations;
    bool disableLinewrap;
    bool hideCursor;
    bool escapeBedrock;
    FFBinaryPrefixType binaryPrefixType;
    FFGLType glType;
    bool pipe; //disables logo and all escape sequences
    bool multithreading;
    bool stat;

    FFModuleArgs os;
    FFModuleArgs host;
    FFModuleArgs bios;
    FFModuleArgs board;
    FFModuleArgs brightness;
    FFModuleArgs chassis;
    FFModuleArgs kernel;
    FFModuleArgs uptime;
    FFModuleArgs processes;
    FFModuleArgs packages;
    FFModuleArgs shell;
    FFModuleArgs display;
    FFModuleArgs de;
    FFModuleArgs wallpaper;
    FFModuleArgs wifi;
    FFModuleArgs wm;
    FFModuleArgs wmTheme;
    FFModuleArgs theme;
    FFModuleArgs icons;
    FFModuleArgs font;
    FFModuleArgs cursor;
    FFModuleArgs terminal;
    FFModuleArgs terminalFont;
    FFModuleArgs cpu;
    FFModuleArgs cpuUsage;
    FFModuleArgs gpu;
    FFModuleArgs memory;
    FFModuleArgs swap;
    FFModuleArgs disk;
    FFModuleArgs battery;
    FFModuleArgs powerAdapter;
    FFModuleArgs locale;
    FFModuleArgs localIP;
    FFModuleArgs publicIP;
    FFModuleArgs weather;
    FFModuleArgs player;
    FFModuleArgs media;
    FFModuleArgs dateTime;
    FFModuleArgs date;
    FFModuleArgs time;
    FFModuleArgs vulkan;
    FFModuleArgs openGL;
    FFModuleArgs openCL;
    FFModuleArgs users;
    FFModuleArgs bluetooth;
    FFModuleArgs sound;
    FFModuleArgs gamepad;

    FFstrbuf libPCI;
    FFstrbuf libVulkan;
    FFstrbuf libWayland;
    FFstrbuf libXcbRandr;
    FFstrbuf libXcb;
    FFstrbuf libXrandr;
    FFstrbuf libX11;
    FFstrbuf libGIO;
    FFstrbuf libDConf;
    FFstrbuf libDBus;
    FFstrbuf libXFConf;
    FFstrbuf libSQLite3;
    FFstrbuf librpm;
    FFstrbuf libImageMagick;
    FFstrbuf libZ;
    FFstrbuf libChafa;
    FFstrbuf libEGL;
    FFstrbuf libGLX;
    FFstrbuf libOSMesa;
    FFstrbuf libOpenCL;
    FFstrbuf libcJSON;
    FFstrbuf libfreetype;
    FFstrbuf libPulse;
    FFstrbuf libwlanapi;
    FFstrbuf libnm;

    bool cpuTemp;
    bool gpuTemp;
    bool gpuForceVulkan;
    bool batteryTemp;

    bool gpuHideIntegrated;
    bool gpuHideDiscrete;

    bool titleFQDN;

    bool shellVersion;
    bool terminalVersion;

    FFstrbuf diskFolders;
    bool diskShowRemovable;
    bool diskShowHidden;
    bool diskShowUnknown;
    bool diskShowSubvolumes;

    FFDisplayCompactType displayCompactType;
    bool displayDetectName;
    bool displayPreciseRefreshRate;

    bool bluetoothShowDisconnected;

    FFstrbuf batteryDir;

    FFstrbuf separatorString;

    FFstrbuf localIpNamePrefix;
    FFLocalIpType localIpShowType;

    FFstrbuf publicIpUrl;
    uint32_t publicIpTimeout;

    FFstrbuf weatherOutputFormat;
    uint32_t weatherTimeout;

    FFSoundType soundType;

    FFstrbuf osFile;

    FFstrbuf playerName;

    uint32_t percentType;

    FFstrbuf commandShell;
    FFlist commandKeys;
    FFlist commandTexts;
} FFconfig;

typedef struct FFstate
{
    uint32_t logoWidth;
    uint32_t logoHeight;
    uint32_t keysHeight;

    FFPlatform platform;
} FFstate;

typedef struct FFinstance
{
    FFconfig config;
    FFstate state;
} FFinstance;

//////////////////////
// Init functions //
//////////////////////

//common/init.c
void ffInitInstance(FFinstance* instance);
void ffStart(FFinstance* instance);
void ffFinish(FFinstance* instance);
void ffDestroyInstance(FFinstance* instance);

void ffListFeatures();

////////////////////
// Logo functions //
////////////////////

void ffLogoPrint(FFinstance* instance);
void ffLogoPrintRemaining(FFinstance* instance);
void ffLogoPrintLine(FFinstance* instance);

void ffLogoBuiltinPrint(FFinstance* instance);
void ffLogoBuiltinList();
void ffLogoBuiltinListAutocompletion();

//////////////////////
// Module functions //
//////////////////////

//Common

void ffPrintDateTimeFormat(FFinstance* instance, const char* moduleName, const FFModuleArgs* moduleArgs);
void ffPrepareCPUUsage();
void ffPreparePublicIp(FFinstance* instance);
void ffPrepareWeather(FFinstance* instance);

//Printing

void ffPrintCustom(FFinstance* instance, const char* key, const char* value);
void ffPrintBreak(FFinstance* instance);
void ffPrintTitle(FFinstance* instance);
void ffPrintSeparator(FFinstance* instance);
void ffPrintOS(FFinstance* instance);
void ffPrintHost(FFinstance* instance);
void ffPrintBios(FFinstance* instance);
void ffPrintBoard(FFinstance* instance);
void ffPrintChassis(FFinstance* instance);
void ffPrintKernel(FFinstance* instance);
void ffPrintUptime(FFinstance* instance);
void ffPrintProcesses(FFinstance* instance);
void ffPrintPackages(FFinstance* instance);
void ffPrintShell(FFinstance* instance);
void ffPrintDisplay(FFinstance* instance);
void ffPrintBrightness(FFinstance* instance);
void ffPrintDesktopEnvironment(FFinstance* instance);
void ffPrintWM(FFinstance* instance);
void ffPrintWMTheme(FFinstance* instance);
void ffPrintTheme(FFinstance* instance);
void ffPrintIcons(FFinstance* instance);
void ffPrintWallpaper(FFinstance* instance);
void ffPrintFont(FFinstance* instance);
void ffPrintCursor(FFinstance* instance);
void ffPrintTerminal(FFinstance* instance);
void ffPrintTerminalFont(FFinstance* instance);
void ffPrintCPU(FFinstance* instance);
void ffPrintCPUUsage(FFinstance* instance);
void ffPrintGPU(FFinstance* instance);
void ffPrintMemory(FFinstance* instance);
void ffPrintSwap(FFinstance* instance);
void ffPrintDisk(FFinstance* instance);
void ffPrintBattery(FFinstance* instance);
void ffPrintPowerAdapter(FFinstance* instance);
void ffPrintLocale(FFinstance* instance);
void ffPrintPlayer(FFinstance* instance);
void ffPrintMedia(FFinstance* instance);
void ffPrintDateTime(FFinstance* instance);
void ffPrintDate(FFinstance* instance);
void ffPrintTime(FFinstance* instance);
void ffPrintLocalIp(FFinstance* instance);
void ffPrintPublicIp(FFinstance* instance);
void ffPrintWeather(FFinstance* instance);
void ffPrintWifi(FFinstance* instance);
void ffPrintColors(FFinstance* instance);
void ffPrintVulkan(FFinstance* instance);
void ffPrintOpenGL(FFinstance* instance);
void ffPrintOpenCL(FFinstance* instance);
void ffPrintUsers(FFinstance* instance);
void ffPrintCommand(FFinstance* instance);
void ffPrintBluetooth(FFinstance* instance);
void ffPrintSound(FFinstance* instance);
void ffPrintGamepad(FFinstance* instance);

#endif
