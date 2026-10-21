#include "brightness.h"
#include "common/arrutil.h"
#include "common/io.h"
#include "common/kmod.h"
#include "common/debug.h"
#include "common/time.h"
#include "common/edidHelper.h"

#include <ctype.h>
#include <errno.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/fcntl.h>
#include <unistd.h>

#if __has_include(<dev/iicbus/iic.h>)
    #include <dev/iicbus/iic.h>

    // Try to load cache from file
    static bool loadCacheFromFile(FFBrightnessDdcCache* cache) {
        cache->count = 0;
        FF_STRBUF_AUTO_DESTROY cacheDir = ffBrightnessDdcCachePath();
        FF_STRBUF_AUTO_DESTROY cacheContent = ffStrbufCreate();
        bool valid = false;
        FF_DEBUG("Brightness DDC/CI: loading I2C cache from %s", cacheDir.chars);
        if (ffReadFileBuffer(cacheDir.chars, &cacheContent)) {
            // Parse cache: first number is count, rest are device numbers
            char* ptr = cacheContent.chars;
            char* end = ptr + cacheContent.length;

            // Read count
            char* next;
            long count = strtol(ptr, &next, 10);
            if (next != ptr && count > 0 && count <= FF_I2C_DEVICE_CACHE_MAX) {
                ptr = next;

                // Read device numbers and reject truncated or trailing data.
                size_t parsedCount = 0;
                for (; parsedCount < (size_t) count && ptr < end; ++parsedCount) {
                    long dev = strtol(ptr, &next, 10);
                    if (next == ptr || dev < 0 || dev >= 100) {
                        break;
                    }
                    cache->devices[parsedCount] = (int) dev;
                    ptr = next;
                }

                while (ptr < end && isspace((unsigned char) *ptr)) {
                    ++ptr;
                }

                if (parsedCount == (size_t) count && ptr == end) {
                    cache->count = parsedCount;
                    valid = true;
                    for (size_t i = 0; i < cache->count; ++i) {
                        char path[32];
                        snprintf(path, sizeof(path), "/dev/iic%d", cache->devices[i]);
                        FF_AUTO_CLOSE_FD int fd = open(path, O_RDWR | O_CLOEXEC);
                        if (fd < 0) {
                            int error = errno;
                            FF_DEBUG("Brightness DDC/CI: cached bus %d is unavailable (errno=%d: %s)", cache->devices[i], error, strerror(error));
                            valid = false;
                            break;
                        }
                    }
                }
            }
        }

        if (!valid) {
            FF_DEBUG("Brightness DDC/CI: I2C cache is missing or invalid");
            cache->count = 0;
            return false;
        }

        FF_DEBUG("Brightness DDC/CI: loaded %zu bus(es) from cache", cache->count);
        return true;
    }

    // Save cache to file
    static void saveCacheToFile(const FFBrightnessDdcCache* cache) {
        if (cache->count == 0) {
            return;
        }

        FF_STRBUF_AUTO_DESTROY cacheDir = ffBrightnessDdcCachePath();
        FF_STRBUF_AUTO_DESTROY cacheContent = ffStrbufCreateF("%zu", cache->count);
        for (size_t i = 0; i < cache->count; i++) {
            ffStrbufAppendF(&cacheContent, " %d", cache->devices[i]);
        }

        if (ffWriteFileBuffer(cacheDir.chars, &cacheContent)) {
            FF_DEBUG("Brightness DDC/CI: saved %zu bus(es) to %s", cache->count, cacheDir.chars);
        } else {
            FF_DEBUG("Brightness DDC/CI: failed to save cache to %s", cacheDir.chars);
        }
    }

    static bool readDdcCiResponse(int fd, FFBrightnessOptions* options, uint8_t response[FF_DDC_CI_VCP_RESPONSE_BUFFER_SIZE]) {
        uint8_t request[5];
        ffDdcCiBuildGetVcpRequest(request);

        struct iic_msg msg = {
            .slave = FF_DDC_CI_WRITE_ADDR,
            .flags = IIC_M_WR,
            .len = sizeof(request),
            .buf = request
        };

        struct iic_rdwr_data data = {
            .msgs = &msg,
            .nmsgs = 1
        };

        int ioctlResult = ioctl(fd, I2CRDWR, &data);
        if (ioctlResult < 0) {
            int error = errno;
            FF_DEBUG("Brightness DDC/CI: write request failed (fd=%d, result=%d, errno=%d: %s)", fd, ioctlResult, error, strerror(error));
            return false;
        }

        ffTimeSleep(options->ddcciSleep);

        msg.slave = FF_DDC_CI_WRITE_ADDR;
        msg.flags = IIC_M_RD;
        msg.len = FF_DDC_CI_VCP_RESPONSE_BUFFER_SIZE;
        msg.buf = response;
        ioctlResult = ioctl(fd, I2CRDWR, &data);
        if (ioctlResult < 0) {
            int error = errno;
            FF_DEBUG("Brightness DDC/CI: read response failed (fd=%d, result=%d, errno=%d: %s)", fd, ioctlResult, error, strerror(error));
            return false;
        }

        if (!ffDdcCiValidateGetVcpResponse(response)) {
            FF_DEBUG("Brightness DDC/CI: invalid response (addr=0x%02x, header=0x%02x, type=0x%02x, opcode=0x%02x, checksum=0x%02x)",
                (unsigned) response[0], (unsigned) response[1], (unsigned) response[2], (unsigned) response[4], (unsigned) response[10]);
            return false;
        }

        FF_DEBUG("Brightness DDC/CI: valid response (fd=%d, current=%u, max=%u)", fd,
            ((unsigned) response[8] << 8u) | (unsigned) response[9],
            ((unsigned) response[6] << 8u) | (unsigned) response[7]);
        return true;
    }

    static bool scanI2cDevices(FFBrightnessOptions* options, FFBrightnessDdcCache* cache) {
        FF_DEBUG("Brightness DDC/CI: scanning FreeBSD I2C buses");
        cache->count = 0;

        for (int i = 0; i < 100 && cache->count < FF_I2C_DEVICE_CACHE_MAX; i++) {
            char path[32];
            snprintf(path, sizeof(path), "/dev/iic%d", i);

            FF_AUTO_CLOSE_FD int fd = open(path, O_RDWR | O_CLOEXEC);
            if (fd < 0) {
                continue;
            }

            FF_DEBUG("Brightness DDC/CI: probing %s", path);
            uint8_t response[FF_DDC_CI_VCP_RESPONSE_BUFFER_SIZE];
            if (!readDdcCiResponse(fd, options, response)) {
                continue;
            }

            cache->devices[cache->count++] = i;
            FF_DEBUG("Brightness DDC/CI: found compatible display on %s", path);
        }

        FF_DEBUG("Brightness DDC/CI: scan finished with %zu compatible bus(es)", cache->count);
        return cache->count > 0;
    }

    static bool readEdid(int fd, uint8_t* edidData) {
        uint8_t offset = 0;

        struct iic_msg msgs[2];
        msgs[0].slave = FF_DDC_EDID_WRITE_ADDR;
        msgs[0].flags = IIC_M_WR;
        msgs[0].len = 1;
        msgs[0].buf = &offset;

        msgs[1].slave = FF_DDC_EDID_WRITE_ADDR;
        msgs[1].flags = IIC_M_RD;
        msgs[1].len = 128;
        msgs[1].buf = edidData;

        struct iic_rdwr_data data = {
            .msgs = msgs,
            .nmsgs = 2
        };

        return ioctl(fd, I2CRDWR, &data) >= 0;
    }

    static bool detectCachedDevices(FFBrightnessOptions* options, FFlist* result, FFBrightnessDdcCache* cache) {
        const size_t cachedCount = cache->count;
        const uint32_t resultLength = result->length;
        size_t validCount = 0;

        FF_DEBUG("Brightness DDC/CI: validating %zu cached bus(es)", cachedCount);
        for (size_t i = 0; i < cachedCount; ++i) {
            const int device = cache->devices[i];
            char path[32];
            snprintf(path, sizeof(path), "/dev/iic%d", device);

            FF_AUTO_CLOSE_FD int fd = open(path, O_RDWR | O_CLOEXEC);
            if (fd < 0) {
                int error = errno;
                FF_DEBUG("Brightness DDC/CI: failed to open cached bus %d (errno=%d: %s)", device, error, strerror(error));
                continue;
            }

            uint8_t response[FF_DDC_CI_VCP_RESPONSE_BUFFER_SIZE];
            if (!readDdcCiResponse(fd, options, response)) {
                continue;
            }

            uint32_t current = ((uint32_t) response[8] << 8u) + (uint32_t) response[9];
            uint32_t max = ((uint32_t) response[6] << 8u) + (uint32_t) response[7];
            if (max == 0 || current > max) {
                FF_DEBUG("Brightness DDC/CI: ignoring bus %d with invalid brightness range %u/%u", device, current, max);
                continue;
            }

            cache->devices[validCount++] = device;

            // Try to read EDID for monitor name
            uint8_t edidData[128];
            FF_STRBUF_AUTO_DESTROY name = ffStrbufCreateA(32);
            if (readEdid(fd, edidData)) {
                ffEdidGetName(edidData, &name);
            }
            if (name.length == 0) {
                ffStrbufSetF(&name, "I2C-%d", device);
            }

            FFBrightnessResult* brightness = FF_LIST_ADD(FFBrightnessResult, *result);
            brightness->max = max;
            brightness->min = 0;
            brightness->current = current;
            brightness->builtin = false;
            ffStrbufInitMove(&brightness->name, &name);
            FF_DEBUG("Brightness DDC/CI: added display '%s' from bus %d (%u/%u)", brightness->name.chars, device, current, max);
        }

        cache->count = validCount;
        FF_DEBUG("Brightness DDC/CI: cache validation kept %zu bus(es), result count is %u", validCount, result->length);
        return result->length > resultLength;
    }

    const char* detectWithDdcci(FFBrightnessOptions* options, FFlist* result) {
        FFBrightnessDdcCache cache = {0};
        FF_DEBUG("Brightness DDC/CI: detection started (result count=%u)", result->length);
        if (!loadCacheFromFile(&cache)) {
            if (!scanI2cDevices(options, &cache)) {
                FF_DEBUG("Brightness DDC/CI: no compatible I2C device found during initial scan");
                return "No I2C devices found";
            }
        }

        if (!detectCachedDevices(options, result, &cache)) {
            FF_DEBUG("Brightness DDC/CI: cached detection found no usable display; rescanning");
            if (!scanI2cDevices(options, &cache) || !detectCachedDevices(options, result, &cache)) {
                FF_DEBUG("Brightness DDC/CI: rescan found no compatible display");
                return "No DDC/CI compatible displays found";
            }
        }

        saveCacheToFile(&cache);
        FF_DEBUG("Brightness DDC/CI: detection finished with %u result(s)", result->length);
        return nullptr;
    }

#else

const char* detectWithDdcci([[maybe_unused]] FFBrightnessOptions* options, [[maybe_unused]] FFlist* result) {
    FF_DEBUG("DDC/CI support is not available on this system");
    return "DDC/CI is supported only on FreeBSD";
}

#endif

#if __has_include(<sys/backlight.h>)
    #include <sys/backlight.h>

const char* detectWithBacklight([[maybe_unused]] FFBrightnessOptions* options, FFlist* result) {
    // https://man.freebsd.org/cgi/man.cgi?query=backlight&sektion=9
    char path[] = "/dev/backlight/backlight0";

    for (char i = '0'; i <= '9'; ++i) {
        path[ARRAY_SIZE(path) - 2] = i;

        FF_AUTO_CLOSE_FD int fd = open(path, O_RDONLY | O_CLOEXEC);
        if (fd < 0) {
            int err = errno;
            if (err == ENOENT) {
                FF_DEBUG("No more backlight devices");
                break;
            } else if (i == '0' && !ffKmodLoaded("backlight")) {
                FF_DEBUG("backlight module is not loaded");
                return "No backlight devices found and backlight module is not loaded";
            }
            FF_DEBUG("open(/dev/backlight/backlight%c) failed: %s", i, strerror(errno));
            continue;
        }

        struct backlight_props status;
        if (ioctl(fd, BACKLIGHTGETSTATUS, &status) < 0) {
            continue;
        }

        FFBrightnessResult* brightness = FF_LIST_ADD(FFBrightnessResult, *result);
        ffStrbufInit(&brightness->name);

        brightness->max = BACKLIGHTMAXLEVELS;
        brightness->min = 0;
        brightness->current = status.brightness;
        brightness->builtin = true;

        struct backlight_info info;
        if (ioctl(fd, BACKLIGHTGETINFO, &info) == 0) {
            ffStrbufAppendS(&brightness->name, info.name);
        } else {
            ffStrbufAppendS(&brightness->name, path + strlen("/dev/backlight/"));
        }
    }
    return nullptr;
}

#else

const char* detectWithBacklight([[maybe_unused]] FFBrightnessOptions* options, [[maybe_unused]] FFlist* result) {
    FF_DEBUG("Backlight support is not available on this system");
    return "Backlight is supported only on FreeBSD 13 and newer";
}

#endif

const char* ffDetectBrightness([[maybe_unused]] FFBrightnessOptions* options, FFlist* result) {
    FF_DEBUG("Brightness: detection started (result count=%u)", result->length);
    detectWithBacklight(options, result);
    FF_DEBUG("Brightness: backlight detection finished (result count=%u)", result->length);

    if (options->ddcciSleep != FF_BRIGHTNESS_DDCCI_SLEEP_SKIP && result->length == 0) {
        FF_DEBUG("Brightness: trying DDC/CI because no backlight result was found");
        detectWithDdcci(options, result);
    } else if (options->ddcciSleep == FF_BRIGHTNESS_DDCCI_SLEEP_SKIP) {
        FF_DEBUG("Brightness: skipping DDC/CI because it is disabled");
    } else {
        FF_DEBUG("Brightness: skipping DDC/CI because backlight detection already found %u result(s)", result->length);
    }
    FF_DEBUG("Brightness: detection finished (result count=%u)", result->length);
    return nullptr;
}
