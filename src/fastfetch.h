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
    //General
    FFlogo logo;
    uint16_t logo_spacer;
    char seperator[16];
    int16_t offsetx;
    char color[32];
    uint8_t titleLength;
    bool colorLogo;
    bool showErrors;
    bool recache;
    bool cacheSave; //This is only set to true when using arguments, because we dont want so save this values

    //OS
    bool osShowArchitecture;

    //Shell
    bool shellShowPath;

    //Battery
    char batteryFormat[32];
    bool batteryShowManufacturer;
    bool batteryShowModel;
    bool batteryShowTechnology;
    bool batteryShowCapacity;
    bool batteryShowStatus;

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
void ffDefaultConfig(FFconfig* config);
void ffPrintKey(FFconfig* config, const char* key);
void ffPrintLogoAndKey(FFinstance* instance, const char* key);
void ffGetFileContent(const char* fileName, char* buffer, uint32_t bufferSize);
void ffParsePropFile(const char* file, const char* regex, char* buffer);
void ffParsePropFileHome(FFinstance* instance, const char* relativeFile, const char* regex, char* buffer);
void ffPrintGtkPretty(const char* gtk2, const char* gtk3, const char* gtk4);
void ffPrintError(FFinstance* instance, const char* key, const char* message);
void ffTrimTrailingWhitespace(char* buffer);
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
void ffPrintCPU(FFinstance* instance);
void ffPrintGPU(FFinstance* instance);
void ffPrintMemory(FFinstance* instance);
void ffPrintDisk(FFinstance* instance);
void ffPrintBattery(FFinstance* instance);
void ffPrintLocale(FFinstance* instance);
void ffPrintColors(FFinstance* instance);

#endif