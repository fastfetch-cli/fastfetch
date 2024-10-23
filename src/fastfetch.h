#pragma once

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
#include "options/display.h"
#include "options/general.h"

#ifdef __has_builtin
    #if __has_builtin(__builtin_types_compatible_p)
        #define ARRAY_SIZE(x) ({ static_assert(!__builtin_types_compatible_p(__typeof__(x), __typeof__(&*(x))), "Must not be a pointer"); sizeof(x) / sizeof(*(x)); })
    #endif
#endif
#ifndef ARRAY_SIZE
    #define ARRAY_SIZE(x) (sizeof(x) / sizeof(*(x)))
#endif


typedef struct FFconfig
{
    FFOptionsLogo logo;
    FFOptionsDisplay display;
    FFOptionsGeneral general;
    FFOptionsModules modules;
} FFconfig;

typedef struct FFstate
{
    uint32_t logoWidth;
    uint32_t logoHeight;
    uint32_t keysHeight;
    bool terminalLightTheme;

    FFPlatform platform;
    yyjson_doc* configDoc;
    yyjson_mut_doc* resultDoc;
    FFstrbuf genConfigPath;
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
