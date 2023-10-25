#pragma once

#ifndef FASTFETCH_INCLUDED
#define FASTFETCH_INCLUDED

#include "fastfetch_config.h"

#include <stdint.h>
#include <stdbool.h>

#ifdef FF_USE_SYSTEM_YYJSON
    #include <yyjson.h>
#else
    #include "3rdparty/yyjson/yyjson.h"
#endif

#ifdef _MSC_VER
    #define __attribute__(x)
#endif

#include "util/FFstrbuf.h"
#include "util/FFlist.h"
#include "util/platform/FFPlatform.h"
#include "util/unused.h"

#include "modules/options.h"
#include "options/logo.h"
#include "options/general.h"

typedef enum FFBinaryPrefixType
{
    FF_BINARY_PREFIX_TYPE_IEC,   // 1024 Bytes = 1 KiB, 1024 KiB = 1 MiB, ... (standard)
    FF_BINARY_PREFIX_TYPE_SI,    // 1000 Bytes = 1 KB, 1000 KB = 1 MB, ...
    FF_BINARY_PREFIX_TYPE_JEDEC, // 1024 Bytes = 1 kB, 1024 K = 1 MB, ...
} FFBinaryPrefixType;

typedef enum FFTemperatureUnit
{
    FF_TEMPERATURE_UNIT_CELSIUS,
    FF_TEMPERATURE_UNIT_FAHRENHEIT,
    FF_TEMPERATURE_UNIT_KELVIN,
} FFTemperatureUnit;

typedef struct FFconfig
{
    FFOptionsLogo logo;
    FFOptionsGeneral general;

    //If one of those is empty, ffLogoPrint will set them
    FFstrbuf colorKeys;
    FFstrbuf colorTitle;

    bool brightColor;

    FFstrbuf keyValueSeparator;

    bool stat;
    bool pipe; //disables logo and all escape sequences
    bool showErrors;
    bool disableLinewrap;
    bool hideCursor;
    FFBinaryPrefixType binaryPrefixType;
    uint8_t sizeNdigits;
    uint8_t sizeMaxPrefix;
    FFTemperatureUnit temperatureUnit;
    FFstrbuf barCharElapsed;
    FFstrbuf barCharTotal;
    uint8_t barWidth;
    bool barBorder;
    uint8_t percentType;
    uint8_t percentNdigits;
    bool noBuffer;
    uint32_t keyWidth;

    FFBatteryOptions battery;
    FFBiosOptions bios;
    FFBluetoothOptions bluetooth;
    FFBoardOptions board;
    FFBreakOptions break_;
    FFBrightnessOptions brightness;
    FFCPUOptions cpu;
    FFCPUUsageOptions cpuUsage;
    FFChassisOptions chassis;
    FFColorsOptions colors;
    FFCommandOptions command;
    FFCursorOptions cursor;
    FFCustomOptions custom;
    FFDEOptions de;
    FFDateTimeOptions dateTime;
    FFDiskOptions disk;
    FFDiskIOOptions diskIo;
    FFDisplayOptions display;
    FFFontOptions font;
    FFGPUOptions gpu;
    FFGamepadOptions gamepad;
    FFHostOptions host;
    FFIconsOptions icons;
    FFKernelOptions kernel;
    FFLMOptions lm;
    FFLocalIpOptions localIP;
    FFLocaleOptions locale;
    FFMediaOptions media;
    FFMemoryOptions memory;
    FFMonitorOptions monitor;
    FFNetIOOptions netIo;
    FFOSOptions os;
    FFOpenCLOptions openCL;
    FFOpenGLOptions openGL;
    FFPackagesOptions packages;
    FFPlayerOptions player;
    FFPowerAdapterOptions powerAdapter;
    FFProcessesOptions processes;
    FFPublicIpOptions publicIP;
    FFSeparatorOptions separator;
    FFShellOptions shell;
    FFSoundOptions sound;
    FFSwapOptions swap;
    FFTerminalFontOptions terminalFont;
    FFTerminalOptions terminal;
    FFTerminalSizeOptions terminalSize;
    FFThemeOptions theme;
    FFTitleOptions title;
    FFUptimeOptions uptime;
    FFUsersOptions users;
    FFVersionOptions version;
    FFVulkanOptions vulkan;
    FFWMOptions wm;
    FFWMThemeOptions wmTheme;
    FFWallpaperOptions wallpaper;
    FFWeatherOptions weather;
    FFWifiOptions wifi;

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
    FFstrbuf libfreetype;
    FFstrbuf libPulse;
    FFstrbuf libnm;
    FFstrbuf libDdcutil;
} FFconfig;

typedef struct FFstate
{
    uint32_t logoWidth;
    uint32_t logoHeight;
    uint32_t keysHeight;

    FFPlatform platform;
    yyjson_doc* configDoc;
    yyjson_mut_doc* resultDoc;
    yyjson_mut_doc* migrateConfigDoc;
} FFstate;

typedef struct FFinstance
{
    FFconfig config;
    FFstate state;
} FFinstance;
extern FFinstance instance; // Defined in `common/init.c`
extern FFModuleBaseInfo** ffModuleInfos[];

//////////////////////
// Init functions //
//////////////////////

//common/init.c
void ffInitInstance();
void ffStart();
void ffFinish();
void ffDestroyInstance();

void ffListFeatures();

////////////////////
// Logo functions //
////////////////////

void ffLogoPrint();
void ffLogoPrintRemaining();
void ffLogoPrintLine();

void ffLogoBuiltinPrint();
void ffLogoBuiltinList();
void ffLogoBuiltinListAutocompletion();

#endif
