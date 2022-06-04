#pragma once

#ifndef FASTFETCH_INCLUDED
#define FASTFETCH_INCLUDED

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <pwd.h>
#include <sys/types.h>
#include <sys/utsname.h>
#include <sys/sysinfo.h>
#include <sys/stat.h>
#include <dlfcn.h>

#include "fastfetch_config.h"

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
    FF_GL_TYPE_GLX
} FFGLType;

typedef struct FFconfig
{
    FFstrbuf logoSource;
    FFLogoType logoType;
    FFstrbuf logoColors[FASTFETCH_LOGO_MAX_COLORS];
    uint32_t logoWidth;
    uint32_t logoHeight;
    uint32_t logoPaddingLeft;
    uint32_t logoPaddingRight;
    bool logoPrintRemaining;

    FFstrbuf mainColor; //If this is empty, ffPrintLogo will set it to the main color of the logo
    FFstrbuf separator;

    bool showErrors;
    bool recache;
    bool cacheSave;
    bool allowSlowOperations;
    bool disableLinewrap;
    bool hideCursor;
    bool escapeBedrock;
    FFGLType glType;

    FFstrbuf osFormat;
    FFstrbuf osKey;
    FFstrbuf hostFormat;
    FFstrbuf hostKey;
    FFstrbuf kernelFormat;
    FFstrbuf kernelKey;
    FFstrbuf uptimeFormat;
    FFstrbuf uptimeKey;
    FFstrbuf processesFormat;
    FFstrbuf processesKey;
    FFstrbuf packagesFormat;
    FFstrbuf packagesKey;
    FFstrbuf shellFormat;
    FFstrbuf shellKey;
    FFstrbuf resolutionFormat;
    FFstrbuf resolutionKey;
    FFstrbuf deFormat;
    FFstrbuf deKey;
    FFstrbuf wmFormat;
    FFstrbuf wmKey;
    FFstrbuf wmThemeFormat;
    FFstrbuf wmThemeKey;
    FFstrbuf themeFormat;
    FFstrbuf themeKey;
    FFstrbuf iconsFormat;
    FFstrbuf iconsKey;
    FFstrbuf fontFormat;
    FFstrbuf fontKey;
    FFstrbuf cursorFormat;
    FFstrbuf cursorKey;
    FFstrbuf terminalFormat;
    FFstrbuf terminalKey;
    FFstrbuf termFontFormat;
    FFstrbuf termFontKey;
    FFstrbuf cpuFormat;
    FFstrbuf cpuKey;
    FFstrbuf cpuUsageFormat;
    FFstrbuf cpuUsageKey;
    FFstrbuf gpuFormat;
    FFstrbuf gpuKey;
    FFstrbuf memoryFormat;
    FFstrbuf memoryKey;
    FFstrbuf diskFormat;
    FFstrbuf diskKey;
    FFstrbuf batteryFormat;
    FFstrbuf batteryKey;
    FFstrbuf localeFormat;
    FFstrbuf localeKey;
    FFstrbuf localIpKey;
    FFstrbuf localIpFormat;
    FFstrbuf publicIpKey;
    FFstrbuf publicIpFormat;
    FFstrbuf playerKey;
    FFstrbuf playerFormat;
    FFstrbuf songKey;
    FFstrbuf songFormat;
    FFstrbuf dateTimeKey;
    FFstrbuf dateTimeFormat;
    FFstrbuf dateKey;
    FFstrbuf dateFormat;
    FFstrbuf timeKey;
    FFstrbuf timeFormat;
    FFstrbuf vulkanKey;
    FFstrbuf vulkanFormat;
    FFstrbuf openGLKey;
    FFstrbuf openGLFormat;

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
    FFstrbuf libGL;
    FFstrbuf libEGL;
    FFstrbuf libGLX;

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

typedef struct FFVersion
{
    uint32_t major;
    uint32_t minor;
    uint32_t patch;
} FFVersion;
#define FF_VERSION_INIT ((FFVersion) {0})

typedef struct FFTitleResult
{
    FFstrbuf userName;
    FFstrbuf hostname;
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

typedef struct FFPlasmaResult
{
    FFstrbuf widgetStyle;
    FFstrbuf colorScheme;
    FFstrbuf icons;
    FFstrbuf font;
} FFPlasmaResult;

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
    FFstrbuf value;
    FFstrbuf deviceClass;
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

typedef enum FFformatargtype
{
    FF_FORMAT_ARG_TYPE_NULL = 0,
    FF_FORMAT_ARG_TYPE_UINT,
    FF_FORMAT_ARG_TYPE_UINT16,
    FF_FORMAT_ARG_TYPE_UINT8,
    FF_FORMAT_ARG_TYPE_INT,
    FF_FORMAT_ARG_TYPE_STRING,
    FF_FORMAT_ARG_TYPE_STRBUF,
    FF_FORMAT_ARG_TYPE_DOUBLE,
    FF_FORMAT_ARG_TYPE_LIST
} FFformatargtype;

typedef struct FFformatarg
{
    FFformatargtype type;
    const void* value;
} FFformatarg;

typedef struct FFcache
{
    FILE* value;
    FILE* split;
} FFcache;

typedef enum FFvarianttype
{
    FF_VARIANT_TYPE_STRING,
    FF_VARIANT_TYPE_BOOL,
    FF_VARIANT_TYPE_INT
} FFvarianttype;

typedef union FFvariant
{
    const char* strValue;
    int32_t intValue;
    struct
    {
        bool boolValueSet;
        bool boolValue;
    };
} FFvariant;

#define FF_VARIANT_NULL ((FFvariant){.strValue = NULL})

typedef struct FFfont
{
    FFstrbuf pretty;
    FFstrbuf name;
    FFstrbuf size;
    FFlist styles;
} FFfont;

typedef struct FFpropquery
{
    const char* start;
    FFstrbuf* buffer;
} FFpropquery;

typedef enum FFInitState
{
    FF_INITSTATE_UNINITIALIZED = 0,
    FF_INITSTATE_SUCCESSFUL = 1,
    FF_INITSTATE_FAILED = 2
} FFInitState;

#define FF_LIBRARY_SYMBOL(symbolName) \
    __typeof__(&symbolName) ff ## symbolName;

#define FF_LIBRARY_LOAD(libraryObjectName, userLibraryName, returnValue, ...) \
    void* libraryObjectName =  ffLibraryLoad(&userLibraryName, __VA_ARGS__, NULL);\
    if(libraryObjectName == NULL) \
        return returnValue;

#define FF_LIBRARY_LOAD_SYMBOL_ADRESS(library, symbolMapping, symbolName, returnValue) \
    symbolMapping = dlsym(library, #symbolName); \
    if(symbolMapping == NULL) \
    { \
        dlclose(library); \
        return returnValue; \
    }

#define FF_LIBRARY_LOAD_SYMBOL(library, symbolName, returnValue) \
    __typeof__(&symbolName) FF_LIBRARY_LOAD_SYMBOL_ADRESS(library, ff ## symbolName, symbolName, returnValue);

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

//common/io.c
void ffAppendFDContent(int fd, FFstrbuf* buffer);
bool ffAppendFileContent(const char* fileName, FFstrbuf* buffer); //returns true if open() succeeds. This is used to differentiate between <file not found> and <empty file>
bool ffGetFileContent(const char* fileName, FFstrbuf* buffer);
bool ffWriteFDContent(int fd, const FFstrbuf* content);
bool ffWriteFileContent(const char* fileName, const FFstrbuf* buffer);

bool ffFileExists(const char* fileName, mode_t mode);
void ffSuppressIO(bool suppress); // Not thread safe!

void ffGetTerminalResponse(const char* request, const char* format, ...);

//common/printing.c
void ffPrintError(FFinstance* instance, const char* moduleName, uint8_t moduleIndex, const FFstrbuf* customKeyFormat, const FFstrbuf* formatString, uint32_t numFormatArgs, const char* message, ...);
void ffPrintFormatString(FFinstance* instance, const char* moduleName, uint8_t moduleIndex, const FFstrbuf* customKeyFormat, const FFstrbuf* formatString, const FFstrbuf* error, uint32_t numArgs, const FFformatarg* arguments);
void ffPrintColor(const FFstrbuf* colorValue);
void ffPrintCharTimes(char c, uint32_t times);

//common/caching.c
void ffCacheValidate(FFinstance* instance);

void ffCacheOpenWrite(FFinstance* instance, const char* moduleName, FFcache* cache);
void ffCacheClose(FFcache* cache);

bool ffPrintFromCache(FFinstance* instance, const char* moduleName, const FFstrbuf* customKeyFormat, const FFstrbuf* formatString, uint32_t numArgs);
void ffPrintAndAppendToCache(FFinstance* instance, const char* moduleName, uint8_t moduleIndex, const FFstrbuf* customKeyFormat, FFcache* cache, const FFstrbuf* value, const FFstrbuf* formatString, uint32_t numArgs, const FFformatarg* arguments);
void ffPrintAndWriteToCache(FFinstance* instance, const char* moduleName, const FFstrbuf* customKeyFormat, const FFstrbuf* value, const FFstrbuf* formatString, uint32_t numArgs, const FFformatarg* arguments);

//common/properties.c
bool ffParsePropLine(const char* line, const char* start, FFstrbuf* buffer);
bool ffParsePropLines(const char* lines, const char* start, FFstrbuf* buffer);
bool ffParsePropFileValues(const char* filename, uint32_t numQueries, FFpropquery* queries);
bool ffParsePropFile(const char* filename, const char* start, FFstrbuf* buffer);
bool ffParsePropFileHomeValues(const FFinstance* instance, const char* relativeFile, uint32_t numQueries, FFpropquery* queries);
bool ffParsePropFileHome(const FFinstance* instance, const char* relativeFile, const char* start, FFstrbuf* buffer);
bool ffParsePropFileConfigValues(const FFinstance* instance, const char* relativeFile, uint32_t numQueries, FFpropquery* queries);
bool ffParsePropFileConfig(const FFinstance* instance, const char* relativeFile, const char* start, FFstrbuf* buffer);

//common/font.c
void ffFontInitQt(FFfont* font, const char* data);
void ffFontInitPango(FFfont* font, const char* data);
void ffFontInitCopy(FFfont* font, const char* name);
void ffFontDestroy(FFfont* font);

//common/format.c
void ffFormatAppendFormatArg(FFstrbuf* buffer, const FFformatarg* formatarg);
void ffParseFormatString(FFstrbuf* buffer, const FFstrbuf* formatstr, const FFstrbuf* error, uint32_t numArgs, const FFformatarg* arguments);

//common/parsing.c
bool ffStrSet(const char* str);
void ffParseSemver(FFstrbuf* buffer, const FFstrbuf* major, const FFstrbuf* minor, const FFstrbuf* patch);
void ffParseGTK(FFstrbuf* buffer, const FFstrbuf* gtk2, const FFstrbuf* gtk3, const FFstrbuf* gtk4);

void ffVersionToPretty(const FFVersion* version, FFstrbuf* pretty);
int8_t ffVersionCompare(const FFVersion* version1, const FFVersion* version2);

//common/processing.c
void ffProcessAppendStdOut(FFstrbuf* buffer, char* const argv[]);

//common/libraries.c
void* ffLibraryLoad(const FFstrbuf* userProvidedName, ...);

//common/networking.c
void ffNetworkingGetHttp(const char* host, const char* path, uint32_t timeout, FFstrbuf* buffer);

//common/settings.c
FFvariant ffSettingsGetDConf(FFinstance* instance, const char* key, FFvarianttype type);
FFvariant ffSettingsGetGSettings(FFinstance* instance, const char* schemaName, const char* path, const char* key, FFvarianttype type);
FFvariant ffSettingsGet(FFinstance* instance, const char* dconfKey, const char* gsettingsSchemaName, const char* gsettingsPath, const char* gsettingsKey, FFvarianttype type);
FFvariant ffSettingsGetXFConf(FFinstance* instance, const char* channelName, const char* propertyName, FFvarianttype type);

int ffSettingsGetSQLite3Int(const FFinstance* instance, const char* dbPath, const char* query);

#ifdef __ANDROID__
bool ffSettingsGetAndroidProperty(const char* propName, FFstrbuf* result);
#endif

////////////////////
// Logo functions //
////////////////////

void ffPrintLogo(FFinstance* instance);
void ffPrintRemainingLogo(FFinstance* instance);

void ffPrintLogoLine(FFinstance* instance);
void ffPrintLogoAndKey(FFinstance* instance, const char* moduleName, uint8_t moduleIndex, const FFstrbuf* customKeyFormat);

void ffPrintBuiltinLogos(FFinstance* instance);
void ffListBuiltinLogos();
void ffListBuiltinLogosAutocompletion();

/////////////////////////
// Detection functions //
/////////////////////////

//detection/os.c
const FFOSResult* ffDetectOS(const FFinstance* instance);

//detection/plasma.c
const FFPlasmaResult* ffDetectPlasma(FFinstance* instance);

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
const FFTitleResult* ffDetectTitle(FFinstance* instance);

void ffPrintDateTimeFormat(FFinstance* instance, const char* moduleName, const FFstrbuf* key, const FFstrbuf* format);

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

#endif
