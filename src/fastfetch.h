#pragma once

#ifndef FASTFETCH_INCLUDED
#define FASTFETCH_INCLUDED

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
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
    uint16_t logo_spacer;
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
    FFstrbuf hostFormat;
    FFstrbuf kernelFormat;
    FFstrbuf uptimeFormat;
    FFstrbuf packagesFormat;
    FFstrbuf shellFormat;
    FFstrbuf resolutionFormat;
    FFstrbuf deFormat;
    FFstrbuf wmFormat;
    FFstrbuf themeFormat;
    FFstrbuf iconsFormat;
    FFstrbuf fontFormat;
    FFstrbuf terminalFormat;
    FFstrbuf termFontFormat;
    FFstrbuf cpuFormat;
    FFstrbuf gpuFormat;
    FFstrbuf memoryFormat;
    FFstrbuf diskFormat;
    FFstrbuf batteryFormat;
    FFstrbuf localeFormat;
    
    FFstrbuf libPCI;
    FFstrbuf libX11;
    FFstrbuf libXrandr;

} FFconfig;

typedef struct FFvalue
{
    FFstrbuf value;
    char* error;
    bool calculated;
} FFvalue;

typedef struct FFstate
{
    uint8_t current_row;
    struct passwd* passwd;
    struct utsname utsname;
    struct sysinfo sysinfo;

    FFvalue terminal;
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
    void* value;
} FFformatarg;

//Util functions
void ffInitState(FFstate* state);
void ffDefaultConfig(FFconfig* config);
void ffPrintKey(FFconfig* config, const char* key);
void ffPrintLogoAndKey(FFinstance* instance, const char* key);
void ffGetFileContent(const char* fileName, FFstrbuf* buffer);
void ffParsePropFile(const char* file, const char* regex, char* buffer);
void ffParsePropFileHome(FFinstance* instance, const char* relativeFile, const char* regex, char* buffer);
void ffParseFont(char* font, char* buffer);
void ffFormatGtkPretty(FFstrbuf* buffer, const char* gtk2, const char* gtk3, const char* gtk4);
void ffPrintError(FFinstance* instance, const char* key, const char* message, ...);
void ffTrimTrailingWhitespace(char* buffer);
bool ffPrintCachedValue(FFinstance* instance, const char* key);
void ffPrintAndSaveCachedValue(FFinstance* instance, const char* key, const char* value);
void ffParseFormatString(FFstrbuf* buffer, FFstrbuf* formatstr, uint32_t numArgs, ...);
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

//Module functions

void ffPopulateTerminal(FFinstance* instance);

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