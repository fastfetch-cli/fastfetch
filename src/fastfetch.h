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
#include <sys/utsname.h>
#include <sys/sysinfo.h>
#include <dlfcn.h>

#include "fastfetch_config.h"

#include "util/FFstrbuf.h"
#include "util/FFlist.h"

static inline void ffUnused(int dummy, ...) { (void) dummy; }
#define FF_UNUSED(...) ffUnused(0, __VA_ARGS__);

#define FASTFETCH_TEXT_MODIFIER_BOLT  "\033[1m"
#define FASTFETCH_TEXT_MODIFIER_ERROR "\033[1;31m"
#define FASTFETCH_TEXT_MODIFIER_RESET "\033[0m"

typedef struct FFlogo
{
    const char** names; //Null terminated
    char* lines;
    const char** colors; // colors[0] is used as key color
    bool isFromUser;
} FFlogo;

typedef struct FFconfig
{
    const FFlogo* logo;

    uint16_t logoKeySpacing;
    FFstrbuf separator;
    int16_t offsetx;
    FFstrbuf color;
    bool colorLogo;
    bool showErrors;
    bool recache;
    bool cacheSave;
    bool printRemainingLogo;
    bool allowSlowOperations;
    bool disableLinewrap;
    bool hideCursor;

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

    FFstrbuf libPCI;
    FFstrbuf libVulkan;
    FFstrbuf libX11;
    FFstrbuf libXrandr;
    FFstrbuf libWayland;
    FFstrbuf libGIO;
    FFstrbuf libDConf;
    FFstrbuf libXFConf;
    FFstrbuf librpm;

    FFstrbuf diskFolders;

    FFstrbuf batteryDir;

    FFstrbuf separatorString;

    bool localIpShowLoop;
    bool localIpShowIpV4;
    bool localIpShowIpV6;

} FFconfig;

typedef struct FFstate
{
    uint32_t logoWidth;
    char* logoLinesIndex;

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
    FFstrbuf error;
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

typedef struct FFWMDEResult
{
    const char* sessionDesktop;
    FFstrbuf wmProcessName;
    FFstrbuf wmPrettyName;
    FFstrbuf wmProtocolName;
    FFstrbuf deProcessName;
    FFstrbuf dePrettyName;
    FFstrbuf deVersion;
} FFWMDEResult;

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

#define FF_LIBRARY_LOAD(libraryObjectName, libraryName, userLibraryName, returnValue) \
    void* libraryObjectName =  dlopen(userLibraryName.length == 0 ? libraryName : userLibraryName.chars, RTLD_LAZY); \
    if(dlerror() != NULL || libraryObjectName == NULL) \
        return returnValue;

#define FF_LIBRARY_LOAD_SYMBOL_ADRESS(library, symbolMapping, symbolName, returnValue) \
    symbolMapping = dlsym(library, #symbolName); \
    if(dlerror() != NULL || symbolMapping == NULL) \
    { \
        dlclose(library); \
        return returnValue; \
    }

#define FF_LIBRARY_LOAD_SYMBOL(library, symbolName, returnValue) \
    __typeof__(&symbolName) FF_LIBRARY_LOAD_SYMBOL_ADRESS(library, ff ## symbolName, symbolName, returnValue);

/*************************/
/* Common util functions */
/*************************/

//common/init.c
void ffInitInstance(FFinstance* instance);
void ffStart(FFinstance* instance);
void ffFinish(FFinstance* instance);

//common/threading.c
void ffStartDetectionThreads(FFinstance* instance);

//common/io.c
void ffPrintLogoAndKey(FFinstance* instance, const char* moduleName, uint8_t moduleIndex, const FFstrbuf* customKeyFormat);
void ffPrintError(FFinstance* instance, const char* moduleName, uint8_t moduleIndex, const FFstrbuf* customKeyFormat, const FFstrbuf* formatString, uint32_t numFormatArgs, const char* message, ...);
void ffPrintFormatString(FFinstance* instance, const char* moduleName, uint8_t moduleIndex, const FFstrbuf* customKeyFormat, const FFstrbuf* formatString, const FFstrbuf* error, uint32_t numArgs, const FFformatarg* arguments);
void ffGetCacheFilePath(FFinstance* instance, const char* moduleName, const char* extension, FFstrbuf* buffer);
void ffReadCacheFile(FFinstance* instance, const char* moduleName, const char* extension, FFstrbuf* buffer);
void ffWriteCacheFile(FFinstance* instance, const char* moduleName, const char* extension, FFstrbuf* content);
bool ffPrintFromCache(FFinstance* instance, const char* moduleName, const FFstrbuf* customKeyFormat, const FFstrbuf* formatString, uint32_t numArgs);
void ffPrintAndSaveToCache(FFinstance* instance, const char* moduleName, const FFstrbuf* customKeyFormat, const FFstrbuf* value, const FFstrbuf* formatString, uint32_t numArgs, const FFformatarg* arguments);
void ffPrintAndAppendToCache(FFinstance* instance, const char* moduleName, uint8_t moduleIndex, const FFstrbuf* customKeyFormat, FFcache* cache, const FFstrbuf* value, const FFstrbuf* formatString, uint32_t numArgs, const FFformatarg* arguments);

void ffCacheValidate(FFinstance* instance);
void ffCacheOpenWrite(FFinstance* instance, const char* moduleName, FFcache* cache);
void ffCacheClose(FFcache* cache);

void ffAppendFDContent(int fd, FFstrbuf* buffer);
bool ffAppendFileContent(const char* fileName, FFstrbuf* buffer); //returns true if open() succeeds. This is used to differentiate between <file not found> and <empty file>
bool ffGetFileContent(const char* fileName, FFstrbuf* buffer);
bool ffWriteFDContent(int fd, const FFstrbuf* content);
void ffWriteFileContent(const char* fileName, const FFstrbuf* buffer);

// Not thread safe!
void ffSuppressIO(bool suppress);

// They return true if the file was found, independently if start was found
// Buffers which already contain content are not overwritten
// The last occurence of start in the first file will be the one used
bool ffParsePropFileValues(const char* filename, uint32_t numQueries, FFpropquery* queries);
bool ffParsePropFile(const char* filename, const char* start, FFstrbuf* buffer);
bool ffParsePropFileHomeValues(FFinstance* instance, const char* relativeFile, uint32_t numQueries, FFpropquery* queries);
bool ffParsePropFileHome(FFinstance* instance, const char* relativeFile, const char* start, FFstrbuf* buffer);
bool ffParsePropFileConfigValues(FFinstance* instance, const char* relativeFile, uint32_t numQueries, FFpropquery* queries);
bool ffParsePropFileConfig(FFinstance* instance, const char* relativeFile, const char* start, FFstrbuf* buffer);

//common/processing.c
void ffProcessAppendStdOut(FFstrbuf* buffer, char* const argv[]);

//common/logo.c
void ffLoadLogoSet(FFinstance* instance, const char* logo);
void ffLoadLogo(FFinstance* instance);
void ffPrintLogoLine(FFinstance* instance);
void ffPrintRemainingLogo(FFinstance* instance);

#ifndef FASTFETCH_BUILD_FLASHFATCH
    void ffPrintLogos(FFinstance* instance);
    void ffListLogos();
    void ffListLogosForAutocompletion();
#endif

//common/format.c
void ffFormatAppendFormatArg(FFstrbuf* buffer, const FFformatarg* formatarg);
void ffParseFormatString(FFstrbuf* buffer, const FFstrbuf* formatstr, const FFstrbuf* error, uint32_t numArgs, const FFformatarg* arguments);

//common/parsing.c
void ffGetGtkPretty(FFstrbuf* buffer, const FFstrbuf* gtk2, const FFstrbuf* gtk3, const FFstrbuf* gtk4);

void ffFontInitQt(FFfont* font, const char* data);
void ffFontInitPango(FFfont* font, const char* data);
void ffFontInitCopy(FFfont* font, const char* name);
void ffFontDestroy(FFfont* font);

bool ffGetPropValue(const char* line, const char* start, FFstrbuf* buffer);
bool ffGetPropValueFromLines(const char* lines, const char* start, FFstrbuf* buffer);

void ffParseSemver(FFstrbuf* buffer, const FFstrbuf* major, const FFstrbuf* minor, const FFstrbuf* patch);

//common/settings.c
FFvariant ffSettingsGetDConf(FFinstance* instance, const char* key, FFvarianttype type);
FFvariant ffSettingsGetGSettings(FFinstance* instance, const char* schemaName, const char* path, const char* key, FFvarianttype type);
FFvariant ffSettingsGet(FFinstance* instance, const char* dconfKey, const char* gsettingsSchemaName, const char* gsettingsPath, const char* gsettingsKey, FFvarianttype type);
FFvariant ffSettingsGetXFConf(FFinstance* instance, const char* channelName, const char* propertyName, FFvarianttype type);

#ifdef __ANDROID__
void ffSettingsGetAndroidProperty(const char* propName, FFstrbuf* result);
#endif

//common/detectPlasma.c
const FFPlasmaResult* ffDetectPlasma(FFinstance* instance);

//common/detectGTK.c
const FFGTKResult* ffDetectGTK2(FFinstance* instance);
const FFGTKResult* ffDetectGTK4(FFinstance* instance);
const FFGTKResult* ffDetectGTK3(FFinstance* instance);

//common/detectWMDE.c
const FFWMDEResult* ffDetectWMDE(FFinstance* instance);

//common/detectTerminalShell.c
const FFTerminalShellResult* ffDetectTerminalShell(FFinstance* instance);

/********************/
/* Module functions */
/********************/

//Common
const FFOSResult* ffDetectOS(FFinstance* instance);
const FFTitleResult* ffDetectTitle(FFinstance* instance);

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
void ffPrintLocalIp(FFinstance* instance);
void ffPrintColors(FFinstance* instance);

#endif
