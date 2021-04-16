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

#include "fastfetch_config.h"

#include "util/FFstrbuf.h"

#define UNUSED(...) (void)(__VA_ARGS__)

#define FASTFETCH_TEXT_MODIFIER_BOLT  "\033[1m"
#define FASTFETCH_TEXT_MODIFIER_ERROR "\033[1;31m"
#define FASTFETCH_TEXT_MODIFIER_RESET "\033[0m"

typedef struct FFlogo
{
    uint8_t width;
    uint8_t height;
    char    chars[256][256];
    char    name[128];
    char    color[32];
} FFlogo;

typedef struct FFconfig
{
    FFlogo logo;
    uint16_t logo_spacing;
    FFstrbuf seperator;
    int16_t offsetx;
    FFstrbuf color;
    uint32_t titleLength;
    bool colorLogo;
    bool showErrors;
    bool recache;
    bool cacheSave; //This is only set to true when using arguments, because we dont want so save this values
    bool printRemainingLogo;

    FFstrbuf osFormat;
    FFstrbuf osKey;
    FFstrbuf hostFormat;
    FFstrbuf hostKey;
    FFstrbuf kernelFormat;
    FFstrbuf kernelKey;
    FFstrbuf uptimeFormat;
    FFstrbuf uptimeKey;
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
    FFstrbuf terminalFormat;
    FFstrbuf terminalKey;
    FFstrbuf termFontFormat;
    FFstrbuf termFontKey;
    FFstrbuf cpuFormat;
    FFstrbuf cpuKey;
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

    FFstrbuf libPCI;
    FFstrbuf libX11;
    FFstrbuf libXrandr;
    FFstrbuf libWayland;
    FFstrbuf libDConf;

    FFstrbuf diskFolders;

} FFconfig;

typedef struct FFstate
{
    uint8_t current_row;
    struct passwd* passwd;
    struct utsname utsname;
    struct sysinfo sysinfo;
} FFstate;

typedef struct FFinstance
{
    FFconfig config;
    FFstate state;
} FFinstance;

typedef enum FFformatargtype
{
    FF_FORMAT_ARG_TYPE_UINT,
    FF_FORMAT_ARG_TYPE_UINT8,
    FF_FORMAT_ARG_TYPE_INT,
    FF_FORMAT_ARG_TYPE_STRING,
    FF_FORMAT_ARG_TYPE_STRBUF,
    FF_FORMAT_ARG_TYPE_DOUBLE,
    FF_FORMAT_ARG_TYPE_NULL
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

/*************************/
/* Common util functions */
/*************************/

//common/init.c
void ffInitInstance(FFinstance* instance);
void ffFinish(FFinstance* instance);

//common/threading.c
void ffStartCalculationThreads(FFinstance* instance);

//common/io.c
void ffPrintLogoAndKey(FFinstance* instance, const char* moduleName, uint8_t moduleIndex, const FFstrbuf* customKeyFormat);
void ffPrintError(FFinstance* instance, const char* moduleName, uint8_t moduleIndex, const FFstrbuf* customKeyFormat, const FFstrbuf* formatString, uint32_t numFormatArgs, const char* message, ...);
void ffPrintFormatString(FFinstance* instance, const char* moduleName, uint8_t moduleIndex, const FFstrbuf* customKeyFormat, const FFstrbuf* formatString, const FFstrbuf* error, uint32_t numArgs, const FFformatarg* arguments);
bool ffPrintFromCache(FFinstance* instance, const char* moduleName, const FFstrbuf* customKeyFormat, const FFstrbuf* formatString, uint32_t numArgs);
void ffPrintAndSaveToCache(FFinstance* instance, const char* moduleName, const FFstrbuf* customKeyFormat, const FFstrbuf* value, const FFstrbuf* formatString, uint32_t numArgs, const FFformatarg* arguments);
void ffPrintAndAppendToCache(FFinstance* instance, const char* moduleName, uint8_t moduleIndex, const FFstrbuf* customKeyFormat, FFcache* cache, const FFstrbuf* value, const FFstrbuf* formatString, uint32_t numArgs, const FFformatarg* arguments);

void ffCacheValidate(FFinstance* instance);
void ffCacheOpenWrite(FFinstance* instance, const char* moduleName, FFcache* cache);
void ffCacheClose(FFcache* cache);

void ffAppendFileContent(const char* fileName, FFstrbuf* buffer);
void ffGetFileContent(const char* fileName, FFstrbuf* buffer);

void ffParsePropFile(const char* file, const char* regex, char* buffer);
void ffParsePropFileHome(FFinstance* instance, const char* relativeFile, const char* regex, char* buffer);

//common/logo.c
void ffLoadLogoSet(FFconfig* config, const char* logo);
void ffLoadLogo(FFconfig* config);
void ffPrintLogoLine(FFinstance* instance);
void ffPrintRemainingLogo(FFinstance* instance);

#ifndef FLASHFETCH_BUILD_FLASHFATCH
    void ffListLogos();
    void ffPrintLogos(bool color);
#endif

//common/format.c
void ffFormatAppendFormatArg(FFstrbuf* buffer, const FFformatarg* formatarg);
void ffParseFormatString(FFstrbuf* buffer, const FFstrbuf* formatstr, const FFstrbuf* error, uint32_t numArgs, const FFformatarg* arguments);

//common/parsing.c
void ffGetGtkPretty(FFstrbuf* buffer, FFstrbuf* gtk2, FFstrbuf* gtk3, FFstrbuf* gtk4);
void ffGetFont(const char* font, FFstrbuf* name, double* size);
void ffGetFontPretty(FFstrbuf* buffer, const FFstrbuf* name, double size);

//common/calculatePlasma.c
void ffCalculatePlasma(FFinstance* instance, FFstrbuf** themeNamePtr, FFstrbuf** colorNamePtr, FFstrbuf** iconsNamePtr, FFstrbuf** fontNamePtr);

//common/calculateGTK.c
void ffCalculateGTK2(FFinstance* instance, FFstrbuf** themeNamePtr, FFstrbuf** iconsNamePtr, FFstrbuf** fontNamePtr);
void ffCalculateGTK4(FFinstance* instance, FFstrbuf** themeNamePtr, FFstrbuf** iconsNamePtr, FFstrbuf** fontNamePtr);
void ffCalculateGTK3(FFinstance* instance, FFstrbuf** themeNamePtr, FFstrbuf** iconsNamePtr, FFstrbuf** fontNamePtr);

//common/calculateWM.c
void ffCalculateWM(FFinstance* instance, FFstrbuf** prettyNamePtr, FFstrbuf** processNamePtr, FFstrbuf** errorPtr);

//common/calculateTerminal.c
void ffCalculateTerminal(FFinstance* instance, FFstrbuf** exeNamePtr, FFstrbuf** processNamePtr, FFstrbuf** errorPtr);

/********************/
/* Module functions */
/********************/

void ffPrintCustom(FFinstance* instance, const char* key, const char* value);
void ffPrintBreak(FFinstance* instance);
void ffPrintTitle(FFinstance* instance);
void ffPrintSeperator(FFinstance* instance);
void ffPrintOS(FFinstance* instance);
void ffPrintHost(FFinstance* instance);
void ffPrintKernel(FFinstance* instance);
void ffPrintUptime(FFinstance* instance);
void ffPrintPackages(FFinstance* instance);
void ffPrintShell(FFinstance* instance);
void ffPrintResolution(FFinstance* instance);
void ffPrintDesktopEnvironment(FFinstance* instance);
void ffPrintWM(FFinstance* instance);
void ffPrintWMTheme(FFinstance* instance);
void ffPrintTheme(FFinstance* instance);
void ffPrintIcons(FFinstance* instance);
void ffPrintFont(FFinstance* instance);
void ffPrintTerminal(FFinstance* instance);
void ffPrintTerminalFont(FFinstance* instance);
void ffPrintCPU(FFinstance* instance);
void ffPrintGPU(FFinstance* instance);
void ffPrintMemory(FFinstance* instance);
void ffPrintDisk(FFinstance* instance);
void ffPrintBattery(FFinstance* instance);
void ffPrintLocale(FFinstance* instance);
void ffPrintColors(FFinstance* instance);

#endif
