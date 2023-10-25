#pragma once

#ifndef FASTFETCH_INCLUDED
#define FASTFETCH_INCLUDED

#include "fastfetch_config.h"

#include <stdint.h>
#include <stdbool.h>

#ifdef FF_USE_SYSTEM_YYJSON
    #include <yyjson.h>
#else
    #include "3rdparty/yyjson/yyjson.h"
#endif

#ifdef _MSC_VER
    #define __attribute__(x)
#endif

#include "util/FFstrbuf.h"
#include "util/FFlist.h"
#include "util/platform/FFPlatform.h"
#include "util/unused.h"

#include "options/modules.h"
#include "options/logo.h"
#include "options/general.h"
#include "options/library.h"

typedef enum FFBinaryPrefixType
{
    FF_BINARY_PREFIX_TYPE_IEC,   // 1024 Bytes = 1 KiB, 1024 KiB = 1 MiB, ... (standard)
    FF_BINARY_PREFIX_TYPE_SI,    // 1000 Bytes = 1 KB, 1000 KB = 1 MB, ...
    FF_BINARY_PREFIX_TYPE_JEDEC, // 1024 Bytes = 1 kB, 1024 K = 1 MB, ...
} FFBinaryPrefixType;

typedef enum FFTemperatureUnit
{
    FF_TEMPERATURE_UNIT_CELSIUS,
    FF_TEMPERATURE_UNIT_FAHRENHEIT,
    FF_TEMPERATURE_UNIT_KELVIN,
} FFTemperatureUnit;

typedef struct FFconfig
{
    FFOptionsLogo logo;
    FFOptionsGeneral general;
    FFOptionsModules modules;

    //If one of those is empty, ffLogoPrint will set them
    FFstrbuf colorKeys;
    FFstrbuf colorTitle;

    bool brightColor;

    FFstrbuf keyValueSeparator;

    bool stat;
    bool pipe; //disables logo and all escape sequences
    bool showErrors;
    bool disableLinewrap;
    bool hideCursor;
    FFBinaryPrefixType binaryPrefixType;
    uint8_t sizeNdigits;
    uint8_t sizeMaxPrefix;
    FFTemperatureUnit temperatureUnit;
    FFstrbuf barCharElapsed;
    FFstrbuf barCharTotal;
    uint8_t barWidth;
    bool barBorder;
    uint8_t percentType;
    uint8_t percentNdigits;
    bool noBuffer;
    uint32_t keyWidth;

    FFOptionsLibrary library;
} FFconfig;

typedef struct FFstate
{
    uint32_t logoWidth;
    uint32_t logoHeight;
    uint32_t keysHeight;

    FFPlatform platform;
    yyjson_doc* configDoc;
    yyjson_mut_doc* resultDoc;
    yyjson_mut_doc* migrateConfigDoc;
} FFstate;

typedef struct FFinstance
{
    FFconfig config;
    FFstate state;
} FFinstance;
extern FFinstance instance; // Defined in `common/init.c`
extern FFModuleBaseInfo** ffModuleInfos[];

//////////////////////
// Init functions //
//////////////////////

//common/init.c
void ffInitInstance();
void ffStart();
void ffFinish();
void ffDestroyInstance();

void ffListFeatures();

////////////////////
// Logo functions //
////////////////////

void ffLogoPrint();
void ffLogoPrintRemaining();
void ffLogoPrintLine();

void ffLogoBuiltinPrint();
void ffLogoBuiltinList();
void ffLogoBuiltinListAutocompletion();

#endif
