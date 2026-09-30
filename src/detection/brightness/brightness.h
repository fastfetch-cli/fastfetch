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
    FF_DDC_EDID_WRITE_ADDR = FF_DDC_EDID_ADDR << 1,
    FF_DDC_CI_ADDR = 0x37u,
    FF_DDC_CI_WRITE_ADDR = FF_DDC_CI_ADDR << 1,
    FF_DDC_CI_READ_ADDR = FF_DDC_CI_WRITE_ADDR | 1,
    FF_DDC_CI_VCP_COMMAND = 0x51u,
    FF_DDC_CI_GET_VCP = 0x01u,
    FF_DDC_CI_SET_VCP = 0x03u,
    FF_DDC_CI_COMMAND_PACKET = 0x80u,
    FF_DDC_CI_LUMINANCE_OPCODE = 0x10u,
    FF_DDC_CI_VCP_RESPONSE_LENGTH = 8u,
    FF_DDC_CI_VCP_RESPONSE_PACKET_SIZE = 11u,
    FF_DDC_CI_VCP_RESPONSE_BUFFER_SIZE = 12u,
    FF_DDC_CI_RESPONSE_CHECKSUM_ADDR = 0x50u,
};
#define FF_BRIGHTNESS_DDCCI_SLEEP_SKIP ((uint32_t) -1)
#define FF_DDC_CI_MAKE_HEADER(len) (FF_DDC_CI_COMMAND_PACKET | ((len) & 0x7F))

static inline uint8_t ffDdcCiChecksum(uint8_t address, const uint8_t* data, size_t length) {
    uint8_t checksum = address;
    for (size_t i = 0; i < length; ++i) {
        checksum ^= data[i];
    }
    return checksum;
}

static inline void ffDdcCiBuildGetVcpRequest(uint8_t request[5]) {
    request[0] = FF_DDC_CI_VCP_COMMAND;
    request[1] = FF_DDC_CI_MAKE_HEADER(2);
    request[2] = FF_DDC_CI_GET_VCP;
    request[3] = FF_DDC_CI_LUMINANCE_OPCODE;
    request[4] = ffDdcCiChecksum(FF_DDC_CI_WRITE_ADDR, request, 4);
}

static inline bool ffDdcCiValidateGetVcpResponse(const uint8_t response[FF_DDC_CI_VCP_RESPONSE_BUFFER_SIZE]) {
    if (response[0] != FF_DDC_CI_WRITE_ADDR
        || response[1] != FF_DDC_CI_MAKE_HEADER(FF_DDC_CI_VCP_RESPONSE_LENGTH)
        || response[2] != 0x02u
        || response[3] != 0x00u
        || response[4] != FF_DDC_CI_LUMINANCE_OPCODE
        || response[5] != 0x00u) {
        return false;
    }

    return ffDdcCiChecksum(FF_DDC_CI_RESPONSE_CHECKSUM_ADDR, response, FF_DDC_CI_VCP_RESPONSE_PACKET_SIZE - 1u) == response[FF_DDC_CI_VCP_RESPONSE_PACKET_SIZE - 1u];
}

#define FF_I2C_DEVICE_CACHE_MAX 32

typedef struct FFBrightnessDdcCache {
    int devices[FF_I2C_DEVICE_CACHE_MAX];
    size_t count;
} FFBrightnessDdcCache;

static inline FFstrbuf ffBrightnessDdcCachePath(void) {
    FFstrbuf path = ffStrbufCreateCopy(&instance.state.platform.cacheDir);
    ffStrbufEnsureEndsWithC(&path, '/');
    ffStrbufAppendS(&path, "fastfetch/brightness/i2c_cache.txt");
    return path;
}

const char* ffDetectBrightness(FFBrightnessOptions* options, FFlist* result); // list of FFBrightnessResult
