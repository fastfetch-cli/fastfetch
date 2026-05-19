#pragma once

#include "fastfetch.h"
#include "modules/brightness/option.h"

typedef struct FFBrightnessResult {
    FFstrbuf name;
    double min, max, current;
    bool builtin;
} FFBrightnessResult;

enum {
    FF_DDC_EDID_ADDR = 0x50u,
    FF_DDC_CI_ADDR = 0x37u,
    FF_DDC_CI_WRITE_ADDR = FF_DDC_CI_ADDR << 1,
    FF_DDC_CI_READ_ADDR = FF_DDC_CI_WRITE_ADDR | 1,
    FF_DDC_CI_VCP_COMMAND = 0x51u,
    FF_DDC_CI_GET_VCP = 0x01u,
    FF_DDC_CI_SET_VCP = 0x03u,
    FF_DDC_CI_COMMAND_PACKET = 0x80u,
    FF_DDC_CI_LUMINANCE_OPCODE = 0x10u,
};
#define FF_DDC_CI_MAKE_HEADER(len) (FF_DDC_CI_COMMAND_PACKET | ((len) & 0x7F))

const char* ffDetectBrightness(FFBrightnessOptions* options, FFlist* result); // list of FFBrightnessResult
