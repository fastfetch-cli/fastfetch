#pragma once

#include "fastfetch_config.h"

#include <stdint.h>
#include <stdbool.h>

#include "common/arrutil.h"
#include "common/FFstrbuf.h"
#include "common/FFlist.h"
#include "common/FFPlatform.h"
#include "common/unused.h"

#include "options/logo.h"
#include "options/display.h"
#include "options/general.h"

typedef struct FFconfig {
    FFOptionsLogo logo;
    FFOptionsDisplay display;
    FFOptionsGeneral general;
} FFconfig;

typedef struct FFLogoLineCacheState {
    FFlist lines;
    uint32_t nextLine;
    uint32_t rightOffset;
} FFLogoLineCacheState;

// `--logo-type spin`: kept here instead of in a file-static block so that the
// state lives with the rest of the run and starts clean on every invocation.
typedef struct FFLogoSpinState {
    float degrees; // rotation of the frame drawn last
    double lastDrawMs; // when that frame was drawn
    bool started; // `logo.spin.angle` has been applied
    bool printed; // a frame was drawn, so `ffSpinTick` has work to do
    uint32_t printedWidth;
    uint32_t printedHeight;
} FFLogoSpinState;

typedef struct FFstate {
    uint32_t logoWidth;
    uint32_t logoHeight;
    uint32_t keysHeight;
    bool titleFqdn;
    uint32_t dynamicInterval;
    FFPlatform platform;
    FFLogoLineCacheState logoLineCache;
    FFLogoSpinState logoSpin;
} FFstate;

typedef struct FFinstance {
    FFconfig config;
    FFstate state;
} FFinstance;
extern FFinstance instance; // Defined in `common/init.c`
extern FFModuleBaseInfo** ffModuleInfos[];
