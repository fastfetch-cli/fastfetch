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

#include "modules/options.h"
#include "logo/option.h"

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

typedef struct FFconfig
{
    FFLogoOptions logo;

    //If one of those is empty, ffLogoPrint will set them
    FFstrbuf colorKeys;
    FFstrbuf colorTitle;

    FFstrbuf keyValueSeparator;

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

    FFTitleOptions title;
    FFOSOptions os;
    FFHostOptions host;
    FFBiosOptions bios;
    FFBoardOptions board;
    FFBrightnessOptions brightness;
    FFModuleArgs chassis;
    FFCommandOptions command;
    FFKernelOptions kernel;
    FFUptimeOptions uptime;
    FFModuleArgs processes;
    FFModuleArgs packages;
    FFModuleArgs shell;
    FFDisplayOptions display;
    FFModuleArgs de;
    FFModuleArgs wallpaper;
    FFModuleArgs wifi;
    FFModuleArgs wm;
    FFModuleArgs wmTheme;
    FFModuleArgs theme;
    FFModuleArgs icons;
    FFFontOptions font;
    FFCursorOptions cursor;
    FFModuleArgs terminal;
    FFModuleArgs terminalFont;
    FFCPUOptions cpu;
    FFCPUUsageOptions cpuUsage;
    FFCustomOptions custom;
    FFGPUOptions gpu;
    FFModuleArgs memory;
    FFModuleArgs swap;
    FFDiskOptions disk;
    FFBatteryOptions battery;
    FFModuleArgs powerAdapter;
    FFLocaleOptions locale;
    FFLocalIpOptions localIP;
    FFModuleArgs publicIP;
    FFModuleArgs weather;
    FFModuleArgs player;
    FFModuleArgs media;
    FFDateTimeOptions dateTime;
    FFModuleArgs vulkan;
    FFModuleArgs openGL;
    FFModuleArgs openCL;
    FFModuleArgs users;
    FFBluetoothOptions bluetooth;
    FFSeparatorOptions separator;
    FFSoundOptions sound;
    FFGamepadOptions gamepad;

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
    FFstrbuf libJSONC;
    FFstrbuf libfreetype;
    FFstrbuf libPulse;
    FFstrbuf libwlanapi;
    FFstrbuf libnm;

    bool shellVersion;
    bool terminalVersion;

    FFstrbuf publicIpUrl;
    uint32_t publicIpTimeout;

    FFstrbuf weatherOutputFormat;
    uint32_t weatherTimeout;

    FFstrbuf playerName;

    uint32_t percentType;

    bool jsonConfig;
} FFconfig;

typedef struct FFstate
{
    uint32_t logoWidth;
    uint32_t logoHeight;
    uint32_t keysHeight;
    uint32_t titleLength;

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

void ffPreparePublicIp(FFinstance* instance);
void ffPrepareWeather(FFinstance* instance);

//Printing

void ffPrintChassis(FFinstance* instance);
void ffPrintProcesses(FFinstance* instance);
void ffPrintPackages(FFinstance* instance);
void ffPrintShell(FFinstance* instance);
void ffPrintDesktopEnvironment(FFinstance* instance);
void ffPrintWM(FFinstance* instance);
void ffPrintWMTheme(FFinstance* instance);
void ffPrintTheme(FFinstance* instance);
void ffPrintIcons(FFinstance* instance);
void ffPrintWallpaper(FFinstance* instance);
void ffPrintTerminal(FFinstance* instance);
void ffPrintTerminalFont(FFinstance* instance);
void ffPrintMemory(FFinstance* instance);
void ffPrintSwap(FFinstance* instance);
void ffPrintPowerAdapter(FFinstance* instance);
void ffPrintPlayer(FFinstance* instance);
void ffPrintMedia(FFinstance* instance);
void ffPrintDateTime(FFinstance* instance, FFDateTimeOptions* options);
void ffPrintDate(FFinstance* instance);
void ffPrintTime(FFinstance* instance);
void ffPrintPublicIp(FFinstance* instance);
void ffPrintWeather(FFinstance* instance);
void ffPrintWifi(FFinstance* instance);
void ffPrintColors(FFinstance* instance);
void ffPrintVulkan(FFinstance* instance);
void ffPrintOpenGL(FFinstance* instance);
void ffPrintOpenCL(FFinstance* instance);
void ffPrintUsers(FFinstance* instance);

#endif
