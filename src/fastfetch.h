#pragma once

#ifndef FASTFETCH_INCLUDED
#define FASTFETCH_INCLUDED

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

#include <sys/types.h>
#include <unistd.h>
#include <pwd.h>
#include <sys/utsname.h>
#include <sys/sysinfo.h>

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
    uint16_t logo_seperator;
    int16_t offsetx;
    char color[32];
    uint8_t titleLength;
    bool colorLogo;
    bool showErrors;
    bool recache;
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

//Util functions
void ffInitState(FFstate* state);
void ffPrintKey(FFconfig* config, const char* key);
void ffPrintLogoAndKey(FFinstance* instance, const char* key);
void ffParsePropFile(const char* file, const char* regex, char* buffer);
void ffParsePropFileHome(FFinstance* instance, const char* relativeFile, const char* regex, char* buffer);
void ffPrintGtkPretty(const char* gtk2, const char* gtk3, const char* gtk4);
void ffPrintError(FFinstance* instance, const char* key, const char* message);
bool ffPrintCachedValue(FFinstance* instance, const char* key);
void ffPrintAndSaveCachedValue(FFinstance* instance, const char* key, const char* value);

//Logo functions
void ffLoadLogoSet(FFconfig* config, const char* logo);
void ffLoadLogo(FFconfig* config);
void ffPrintLogoLine(FFinstance* instance);

#ifndef FLASHFETCH_BUILD_FLASHFATCH
void ffListLogos();
void ffPrintLogos(bool color);
#endif

//Module functions
void ffPrintBreak(FFinstance* state);
void ffPrintTitle(FFinstance* state);
void ffPrintSeperator(FFinstance* state);
void ffPrintOS(FFinstance* state);
void ffPrintHost(FFinstance* state);
void ffPrintKernel(FFinstance* state);
void ffPrintUptime(FFinstance* state);
void ffPrintPackages(FFinstance* state);
void ffPrintShell(FFinstance* state);
void ffPrintResolution(FFinstance* state);
void ffPrintDesktopEnvironment(FFinstance* state);
void ffPrintTheme(FFinstance* state);
void ffPrintIcons(FFinstance* state);
void ffPrintFont(FFinstance* state);
void ffPrintTerminal(FFinstance* state);
void ffPrintCPU(FFinstance* state);
void ffPrintGPU(FFinstance* state);
void ffPrintMemory(FFinstance* state);
void ffPrintDisk(FFinstance* state);
void ffPrintBattery(FFinstance* state);
void ffPrintLocale(FFinstance* state);
void ffPrintColors(FFinstance* state);

#endif