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

#include "util/arrayUtils.h"
#include "util/FFstrbuf.h"
#include "util/FFlist.h"
#include "util/platform/FFPlatform.h"
#include "util/unused.h"

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
} FFstate;

typedef struct FFinstance
{
    FFconfig config;
    FFstate state;
} FFinstance;
extern FFinstance instance; // Defined in `common/init.c`
extern FFModuleBaseInfo** ffModuleInfos[];
