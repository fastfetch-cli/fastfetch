#pragma once

#ifndef FASTFETCH_INCLUDED
#define FASTFETCH_INCLUDED

#include "fastfetch_config.h"

#include <stdint.h>
#include <stdbool.h>

#include <pwd.h>
#include <sys/utsname.h>
#include <sys/sysinfo.h>

#include "util/FFstrbuf.h"
#include "util/FFlist.h"
#include "util/FFvaluestore.h"

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

    FFstrbuf mainColor; //If this is empty, ffLogoPrint will set it to the main color of the logo
    FFstrbuf separator;

    bool showErrors;
    bool recache;
    bool cacheSave;
    bool allowSlowOperations;
    bool disableLinewrap;
    bool hideCursor;
    bool escapeBedrock;
    FFGLType glType;
    bool pipe; //disables logo and all escape sequences

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
    FFModuleArgs disk;
    FFModuleArgs battery;
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

    bool titleFQDN;

    FFstrbuf diskFolders;

    FFstrbuf batteryDir;

    FFstrbuf separatorString;

    bool localIpShowLoop;
    bool localIpShowIpV4;
    bool localIpShowIpV6;

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
    struct sysinfo sysinfo;

    FFlist configDirs;
    FFstrbuf cacheDir;
} FFstate;

typedef struct FFinstance
{
    FFconfig config;
    FFstate state;
} FFinstance;

typedef struct FFTitleResult
{
    FFstrbuf userName;
    FFstrbuf hostname;
    FFstrbuf fqdn; //Fully qualified domain name
} FFTitleResult;

typedef struct FFOSResult
{
    FFstrbuf systemName;
    FFstrbuf name;
    FFstrbuf prettyName;
    FFstrbuf id;
    FFstrbuf idLike;
    FFstrbuf variant;
    FFstrbuf variantID;
    FFstrbuf version;
    FFstrbuf versionID;
    FFstrbuf codename;
    FFstrbuf buildID;
    FFstrbuf architecture;
} FFOSResult;

typedef struct FFQtResult
{
    FFstrbuf widgetStyle;
    FFstrbuf colorScheme;
    FFstrbuf icons;
    FFstrbuf font;
} FFQtResult;

typedef struct FFGTKResult
{
    FFstrbuf theme;
    FFstrbuf icons;
    FFstrbuf font;
    FFstrbuf cursor;
    FFstrbuf cursorSize;
} FFGTKResult;

typedef struct FFTerminalShellResult
{
    FFstrbuf shellProcessName;
    FFstrbuf shellExe;
    const char* shellExeName; //pointer to a char in shellExe
    FFstrbuf shellVersion;

    FFstrbuf terminalProcessName;
    FFstrbuf terminalExe;
    const char* terminalExeName; //pointer to a char in terminalExe

    FFstrbuf userShellExe;
    const char* userShellExeName; //pointer to a char in userShellExe
    FFstrbuf userShellVersion;
} FFTerminalShellResult;

typedef struct FFGPUResult
{
    FFstrbuf vendor;
    FFstrbuf name;
    FFstrbuf driver;
    double temperature;
} FFGPUResult;

typedef struct FFVulkanResult
{
    FFstrbuf driver;
    FFstrbuf apiVersion;
    FFstrbuf conformanceVersion;
    FFlist devices; //List of FFGPUResult
} FFVulkanResult;

typedef struct FFResolutionResult
{
    uint32_t width;
    uint32_t height;
    uint32_t refreshRate;
} FFResolutionResult;

typedef struct FFDisplayServerResult
{
    FFstrbuf wmProcessName;
    FFstrbuf wmPrettyName;
    FFstrbuf wmProtocolName;
    FFstrbuf deProcessName;
    FFstrbuf dePrettyName;
    FFstrbuf deVersion;
    FFlist resolutions; //List of FFResolutionResult
} FFDisplayServerResult;

typedef struct FFTempValue
{
    FFstrbuf name;
    FFstrbuf deviceClass;
    double value;
} FFTempValue;

typedef struct FFTempsResult
{
    FFlist values; //List of FFTempValue
} FFTempsResult;

typedef struct FFMediaResult
{
    FFstrbuf busNameShort; //e.g. plasma-browser-integration
    FFstrbuf player; // e.g. Google Chrome
    FFstrbuf song;
    FFstrbuf artist;
    FFstrbuf album;
    FFstrbuf url;
} FFMediaResult;

typedef struct FFDateTimeResult
{
    //Examples for 21.02.2022 - 15:18:37
    uint16_t year; //2022
    uint8_t yearShort; //22
    uint8_t month; //2
    FFstrbuf monthPretty; //02
    FFstrbuf monthName; //February
    FFstrbuf monthNameShort; //Feb
    uint8_t week; //8
    FFstrbuf weekday; //Monday
    FFstrbuf weekdayShort; //Mon
    uint16_t dayInYear; //52
    uint8_t dayInMonth; //21
    uint8_t dayInWeek; //1
    uint8_t hour; //15
    FFstrbuf hourPretty; //15
    uint8_t hour12; //3
    FFstrbuf hour12Pretty; //03
    uint8_t minute; //18
    FFstrbuf minutePretty; //18
    uint8_t second; //37
    FFstrbuf secondPretty; //37
} FFDateTimeResult;

//////////////////////
// Common functions //
//////////////////////

//common/init.c
void ffInitInstance(FFinstance* instance);
void ffStart(FFinstance* instance);
void ffFinish(FFinstance* instance);

void ffListFeatures();

//common/threading.c
void ffStartDetectionThreads(FFinstance* instance);

//common/networking.c
void ffNetworkingGetHttp(const char* host, const char* path, uint32_t timeout, FFstrbuf* buffer);

////////////////////
// Logo functions //
////////////////////

void ffLogoPrint(FFinstance* instance);
void ffLogoPrintRemaining(FFinstance* instance);
void ffLogoPrintLine(FFinstance* instance);

void ffLogoBuiltinPrint(FFinstance* instance);
void ffLogoBuiltinList();
void ffLogoBuiltinListAutocompletion();

/////////////////////////
// Detection functions //
/////////////////////////

//detection/title.c
const FFTitleResult* ffDetectTitle(const FFinstance* instance);

//detection/os.c
const FFOSResult* ffDetectOS(const FFinstance* instance);

//detection/plasma.c
const FFQtResult* ffDetectQt(FFinstance* instance);

//detection/gtk.c
const FFGTKResult* ffDetectGTK2(FFinstance* instance);
const FFGTKResult* ffDetectGTK4(FFinstance* instance);
const FFGTKResult* ffDetectGTK3(FFinstance* instance);

//detection/displayServer.c
const FFDisplayServerResult* ffConnectDisplayServer(const FFinstance* instance);

//detection/terminalShell.c
const FFTerminalShellResult* ffDetectTerminalShell(FFinstance* instance);

//detection/temps.c (currently unused)
const FFTempsResult* ffDetectTemps(const FFinstance* instance);

//detection/media.c
const FFMediaResult* ffDetectMedia(FFinstance* instance);

//detection/dateTime.c
const FFDateTimeResult* ffDetectDateTime(const FFinstance* instance);

//detection/vulkan.c
const FFVulkanResult* ffDetectVulkan(const FFinstance* instance);

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
void ffPrintDisk(FFinstance* instance);
void ffPrintBattery(FFinstance* instance);
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

#endif
