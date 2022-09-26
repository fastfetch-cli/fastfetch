#pragma once

#ifndef FASTFETCH_INCLUDED
#define FASTFETCH_INCLUDED

#include "fastfetch_config.h"

#include <stdint.h>
#include <stdbool.h>

#include <pwd.h>
#include <sys/utsname.h>

#if FF_HAVE_SYSINFO_H
    #include <sys/sysinfo.h>
#endif

#include "util/FFstrbuf.h"
#include "util/FFlist.h"

static inline void ffUnused(int dummy, ...) { (void) dummy; }
#define FF_UNUSED(...) ffUnused(0, __VA_ARGS__);

#define FASTFETCH_TEXT_MODIFIER_BOLT  "\033[1m"
#define FASTFETCH_TEXT_MODIFIER_ERROR "\033[1;31m"
#define FASTFETCH_TEXT_MODIFIER_RESET "\033[0m"

#define FASTFETCH_LOGO_MAX_COLORS 9 //two digits would make parsing much more complicated (index 1 - 9)

typedef enum FFLogoType
{
    FF_LOGO_TYPE_AUTO,    //If something is given, first try builtin, then file. Otherwise detect logo
    FF_LOGO_TYPE_BUILTIN, //Builtin ascii art.
    FF_LOGO_TYPE_FILE,    //Raw text file, printed as is.
    FF_LOGO_TYPE_RAW,     //Raw text file, printed with color codes replacement.
    FF_LOGO_TYPE_SIXEL,   //Image file, printed as sixel codes.
    FF_LOGO_TYPE_KITTY,   //Image file, printed as kitty graphics protocol
    FF_LOGO_TYPE_CHAFA    //Image file, printed as ascii art using libchafa
} FFLogoType;

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

typedef struct FFconfig
{
    struct
    {
        FFstrbuf source;
        FFLogoType type;
        FFstrbuf colors[FASTFETCH_LOGO_MAX_COLORS];
        uint32_t width;
        uint32_t height;
        uint32_t paddingLeft;
        uint32_t paddingRight;
        bool printRemaining;
    } logo;

    //If one of those is empty, ffLogoPrint will set them
    FFstrbuf colorKeys;
    FFstrbuf colorTitle;

    FFstrbuf separator;

    bool showErrors;
    bool recache;
    bool cacheSave;
    bool allowSlowOperations;
    bool disableLinewrap;
    bool hideCursor;
    bool escapeBedrock;
    FFBinaryPrefixType binaryPrefixType;
    FFGLType glType;
    bool pipe; //disables logo and all escape sequences
    bool multithreading;

    FFModuleArgs os;
    FFModuleArgs host;
    FFModuleArgs kernel;
    FFModuleArgs uptime;
    FFModuleArgs processes;
    FFModuleArgs packages;
    FFModuleArgs shell;
    FFModuleArgs resolution;
    FFModuleArgs de;
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
    FFModuleArgs player;
    FFModuleArgs song;
    FFModuleArgs dateTime;
    FFModuleArgs date;
    FFModuleArgs time;
    FFModuleArgs vulkan;
    FFModuleArgs openGL;
    FFModuleArgs openCL;
    FFModuleArgs users;

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
    FFstrbuf libplist;
    FFstrbuf libcJSON;
    FFstrbuf libfreetype;

    bool cpuTemp;
    bool gpuTemp;
    bool batteryTemp;

    bool titleFQDN;

    FFstrbuf diskFolders;
    bool diskRemovable;

    FFstrbuf batteryDir;

    FFstrbuf separatorString;

    bool fontInline;

    bool localIpShowLoop;
    bool localIpShowIpV4;
    bool localIpShowIpV6;
    FFstrbuf localIpNamePrefix;

    uint32_t publicIpTimeout;

    FFstrbuf osFile;

    FFstrbuf playerName;
} FFconfig;

typedef struct FFstate
{
    uint32_t logoWidth;
    uint32_t logoHeight;
    uint32_t keysHeight;

    struct passwd* passwd;
    struct utsname utsname;

    #if FF_HAVE_SYSINFO_H
        struct sysinfo sysinfo;
    #endif

    FFlist configDirs;
    FFstrbuf cacheDir;
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

//Printing

void ffPrintCustom(FFinstance* instance, const char* key, const char* value);
void ffPrintBreak(FFinstance* instance);
void ffPrintTitle(FFinstance* instance);
void ffPrintSeparator(FFinstance* instance);
void ffPrintOS(FFinstance* instance);
void ffPrintHost(FFinstance* instance);
void ffPrintKernel(FFinstance* instance);
void ffPrintUptime(FFinstance* instance);
void ffPrintProcesses(FFinstance* instance);
void ffPrintPackages(FFinstance* instance);
void ffPrintShell(FFinstance* instance);
void ffPrintResolution(FFinstance* instance);
void ffPrintDesktopEnvironment(FFinstance* instance);
void ffPrintWM(FFinstance* instance);
void ffPrintWMTheme(FFinstance* instance);
void ffPrintTheme(FFinstance* instance);
void ffPrintIcons(FFinstance* instance);
void ffPrintFont(FFinstance* instance);
void ffPrintCursor(FFinstance* instance);
void ffPrintTerminal(FFinstance* instance);
void ffPrintTerminalFont(FFinstance* instance);
void ffPrintCPU(FFinstance* instance);
void ffPrintCPUUsage(FFinstance* instance);
void ffPrintGPU(FFinstance* instance);
void ffPrintMemory(FFinstance* instance);
void ffPrintSwap(FFinstance* instance); //Also in modules/memory.c
void ffPrintDisk(FFinstance* instance);
void ffPrintBattery(FFinstance* instance);
void ffPrintPowerAdapter(FFinstance* instance);
void ffPrintLocale(FFinstance* instance);
void ffPrintPlayer(FFinstance* instance);
void ffPrintSong(FFinstance* instance);
void ffPrintDateTime(FFinstance* instance);
void ffPrintDate(FFinstance* instance);
void ffPrintTime(FFinstance* instance);
void ffPrintLocalIp(FFinstance* instance);
void ffPrintPublicIp(FFinstance* instance);
void ffPrintColors(FFinstance* instance);
void ffPrintVulkan(FFinstance* instance);
void ffPrintOpenGL(FFinstance* instance);
void ffPrintOpenCL(FFinstance* instance);
void ffPrintUsers(FFinstance* instance);

#endif
