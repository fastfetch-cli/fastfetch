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

#include "common/arrayUtils.h"
#include "common/FFstrbuf.h"
#include "common/FFlist.h"
#include "common/FFPlatform.h"
#include "common/unused.h"

#include "options/logo.h"
#include "options/display.h"
#include "options/general.h"


typedef struct FFconfig
{
    FFOptionsLogo logo;
    FFOptionsDisplay display;
    FFOptionsGeneral general;
} FFconfig;

typedef struct FFstate
{
    uint32_t logoWidth;
    uint32_t logoHeight;
    uint32_t keysHeight;
    bool terminalLightTheme;
    bool titleFqdn;

    FFPlatform platform;
    yyjson_doc* configDoc;
    yyjson_mut_doc* resultDoc;
    FFstrbuf genConfigPath;
    bool fullConfig;
    uint32_t dynamicInterval;
} FFstate;

typedef struct FFinstance
{
    FFconfig config;
    FFstate state;
} FFinstance;
extern FFinstance instance; // Defined in `common/init.c`
extern FFModuleBaseInfo** ffModuleInfos[];
