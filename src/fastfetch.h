#pragma once

#ifndef FASTFETCH_INCLUDED
#define FASTFETCH_INCLUDED

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include <sys/types.h>
#include <unistd.h>
#include <pwd.h>
#include <sys/utsname.h>
#include <sys/sysinfo.h>

#include "util/FFstrbuf.h"

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
    uint8_t titleLength;
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
    FF_FORMAT_ARG_TYPE_DOUBLE
} FFformatargtype;

typedef struct FFformatarg
{
    FFformatargtype type;
    const void* value;
} FFformatarg;

//Common functions
void ffInitState(FFstate* state);
void ffDefaultConfig(FFconfig* config);
void ffStartCalculationThreads(FFinstance* instance);
void ffPrintKey(FFinstance* instance, FFstrbuf* customKey, const char* defKey);
void ffPrintLogoAndKey(FFinstance* instance, FFstrbuf* customKey, const char* defKey);
void ffPrintError(FFinstance* instance, FFstrbuf* customKey, const char* defKey, const char* message, ...);
bool ffPrintCachedValue(FFinstance* instance, FFstrbuf* customKey, const char* defKey);
void ffPrintAndSaveCachedValue(FFinstance* instance, FFstrbuf* customKey, const char* defKey, FFstrbuf* value);
void ffAppendFileContent(const char* fileName, FFstrbuf* buffer);
void ffGetFileContent(const char* fileName, FFstrbuf* buffer);
void ffParsePropFile(const char* file, const char* regex, char* buffer);
void ffParsePropFileHome(FFinstance* instance, const char* relativeFile, const char* regex, char* buffer);
void ffParseFormatStringV(FFstrbuf* buffer, FFstrbuf* formatstr, uint32_t numArgs, va_list argp);
void ffParseFormatString(FFstrbuf* buffer, FFstrbuf* formatstr, uint32_t numArgs, ...);
void ffPrintFormatString(FFinstance* instance, FFstrbuf* customKey, const char* defKey, FFstrbuf* formatstr, uint32_t numArgs, ...);
void ffFormatGtkPretty(FFstrbuf* buffer, FFstrbuf* gtk2, FFstrbuf* gtk3, FFstrbuf* gtk4);
void ffParseFont(const char* font, FFstrbuf* name, double* size);
void ffFontPretty(FFstrbuf* buffer, const FFstrbuf* name, double size);
void ffFinish(FFinstance* instance);

//Logo functions
void ffLoadLogoSet(FFconfig* config, const char* logo);
void ffLoadLogo(FFconfig* config);
void ffPrintLogoLine(FFinstance* instance);
void ffPrintRemainingLogo(FFinstance* instance);

#ifndef FLASHFETCH_BUILD_FLASHFATCH
void ffListLogos();
void ffPrintLogos(bool color);
#endif

//Helper functions

void ffCalculatePlasma(FFinstance* instance, FFstrbuf** themeNamePtr, FFstrbuf** iconsNamePtr, FFstrbuf** fontNamePtr);
void ffCalculateGTK2(FFinstance* instance, FFstrbuf** themeNamePtr, FFstrbuf** iconsNamePtr, FFstrbuf** fontNamePtr);
void ffCalculateGTK4(FFinstance* instance, FFstrbuf** themeNamePtr, FFstrbuf** iconsNamePtr, FFstrbuf** fontNamePtr);
void ffCalculateGTK3(FFinstance* instance, FFstrbuf** themeNamePtr, FFstrbuf** iconsNamePtr, FFstrbuf** fontNamePtr);
void ffCalculateWM(FFinstance* instance, FFstrbuf** prettyNamePtr, FFstrbuf** processNamePtr, FFstrbuf** errorPtr);
void ffCalculateTerminal(FFinstance* instance, FFstrbuf** exeNamePtr, FFstrbuf** processNamePtr, FFstrbuf** errorPtr);

//Module functions

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
