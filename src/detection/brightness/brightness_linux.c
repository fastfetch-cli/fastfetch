#include "brightness.h"
#include "common/io.h"
#include "common/edidHelper.h"
#include "common/strutil.h"
#include "common/debug.h"
#include "common/time.h"

#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <limits.h>
#include <string.h>

#if defined(__linux__)
    #include <linux/i2c.h>
    #include <linux/i2c-dev.h>
    #include <sys/ioctl.h>
    #include <fcntl.h>
    #include <unistd.h>
    #include <sys/stat.h>

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
                        snprintf(path, sizeof(path), "/dev/i2c-%d", cache->devices[i]);
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

        // i2c_msg.addr is a 7-bit address; the 8-bit write/read addresses are
        // used only for the DDC/CI checksum.
        struct i2c_msg msg = {
            .addr = FF_DDC_CI_ADDR,
            .flags = 0,
            .len = sizeof(request),
            .buf = request
        };

        struct i2c_rdwr_ioctl_data data = {
            .msgs = &msg,
            .nmsgs = 1
        };

        int ioctlResult = ioctl(fd, I2C_RDWR, &data);
        if (ioctlResult != 1) {
            int error = errno;
            FF_DEBUG("Brightness DDC/CI: write request failed (fd=%d, result=%d, errno=%d: %s)", fd, ioctlResult, error, strerror(error));
            return false;
        }

        ffTimeSleep(options->ddcciSleep);

        msg.flags = I2C_M_RD;
        msg.len = FF_DDC_CI_VCP_RESPONSE_BUFFER_SIZE;
        msg.buf = response;
        ioctlResult = ioctl(fd, I2C_RDWR, &data);
        if (ioctlResult != 1) {
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
        FF_DEBUG("Brightness DDC/CI: scanning Linux I2C buses");
        cache->count = 0;

        for (int i = 0; i < 100 && cache->count < FF_I2C_DEVICE_CACHE_MAX; i++) {
            char path[32];
            snprintf(path, sizeof(path), "/dev/i2c-%d", i);

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

        struct i2c_msg msgs[2];
        msgs[0].addr = FF_DDC_EDID_ADDR;
        msgs[0].flags = 0;
        msgs[0].len = 1;
        msgs[0].buf = &offset;

        msgs[1].addr = FF_DDC_EDID_ADDR;
        msgs[1].flags = I2C_M_RD;
        msgs[1].len = 128;
        msgs[1].buf = edidData;

        struct i2c_rdwr_ioctl_data data;
        data.msgs = msgs;
        data.nmsgs = 2;

        return ioctl(fd, I2C_RDWR, &data) == 2;
    }

    static bool detectCachedDevices(FFBrightnessOptions* options, FFlist* result, FFBrightnessDdcCache* cache) {
        const size_t cachedCount = cache->count;
        const uint32_t resultLength = result->length;
        size_t validCount = 0;

        FF_DEBUG("Brightness DDC/CI: validating %zu cached bus(es)", cachedCount);
        for (size_t i = 0; i < cachedCount; ++i) {
            const int device = cache->devices[i];
            char path[32];
            snprintf(path, sizeof(path), "/dev/i2c-%d", device);

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

    static const char* detectWithDdcciNative(FFBrightnessOptions* options, FFlist* result, uint32_t expectedResultLength) {
#ifdef FF_HAVE_DDCUTIL
        const uint32_t resultLength = result->length;
#else
        (void) expectedResultLength;
#endif
        FFBrightnessDdcCache cache = {0};
        FF_DEBUG("Brightness DDC/CI: native detection started (result count=%u, expected=%u)", result->length, expectedResultLength);
        if (!loadCacheFromFile(&cache)) {
            if (!scanI2cDevices(options, &cache)) {
                FF_DEBUG("Brightness DDC/CI: no compatible I2C device found during initial scan");
                return "No I2C devices found";
            }
        }

        if (!detectCachedDevices(options, result, &cache)) {
            FF_DEBUG("Brightness DDC/CI: cached detection found no usable display; rescanning");
            // A cached bus can remain present after its monitor is unplugged or
            // its number is reused. Rebuild the cache when no valid reply arrives.
            if (!scanI2cDevices(options, &cache) || !detectCachedDevices(options, result, &cache)) {
                FF_DEBUG("Brightness DDC/CI: rescan found no compatible display");
                return "No DDC/CI compatible displays found";
            }
        }

    #ifdef FF_HAVE_DDCUTIL
        if (result->length < expectedResultLength) {
            FF_DEBUG("Brightness DDC/CI: native result is incomplete (%u/%u); rolling back for ddcutil fallback", result->length, expectedResultLength);
            FFBrightnessResult discarded;
            while (result->length > resultLength && FF_LIST_POP(*result, &discarded)) {
                ffStrbufDestroy(&discarded.name);
            }
            return "Native DDC/CI found only a subset of displays";
        }
    #endif

        saveCacheToFile(&cache);
        FF_DEBUG("Brightness DDC/CI: native detection finished with %u result(s)", result->length);
        return nullptr;
    }
#endif

static const char* detectWithBacklight(FFlist* result) {
    // https://www.kernel.org/doc/Documentation/ABI/stable/sysfs-class-backlight
    const char* backlightDirPath = "/sys/class/backlight/";

    FF_AUTO_CLOSE_DIR DIR* dirp = opendir(backlightDirPath);
    if (dirp == nullptr) {
        return "Failed to open `/sys/class/backlight/`";
    }

    FF_STRBUF_AUTO_DESTROY backlightDir = ffStrbufCreateA(64);
    ffStrbufAppendS(&backlightDir, backlightDirPath);

    uint32_t backlightDirLength = backlightDir.length;

    FF_STRBUF_AUTO_DESTROY buffer = ffStrbufCreate();

    struct dirent* entry;
    while ((entry = readdir(dirp)) != nullptr) {
        if (entry->d_name[0] == '.') {
            continue;
        }

        ffStrbufAppendS(&backlightDir, entry->d_name);
        ffStrbufAppendS(&backlightDir, "/brightness");
        if (ffReadFileBuffer(backlightDir.chars, &buffer)) {
            double actualBrightness = ffStrbufToDouble(&buffer, 0);
            ffStrbufSubstrBefore(&backlightDir, backlightDirLength);
            ffStrbufAppendS(&backlightDir, entry->d_name);
            ffStrbufAppendS(&backlightDir, "/max_brightness");
            if (ffReadFileBuffer(backlightDir.chars, &buffer)) {
                FFBrightnessResult* brightness = FF_LIST_ADD(FFBrightnessResult, *result);
                ffStrbufSubstrBeforeLastC(&backlightDir, '/');
                ffStrbufAppendS(&backlightDir, "/device");
                ffStrbufInitA(&brightness->name, PATH_MAX);
                if (realpath(backlightDir.chars, brightness->name.chars)) {
                    ffStrbufRecalculateLength(&brightness->name);
                    // if we managed to get edid, use it
                    ffStrbufAppendS(&brightness->name, "/edid");
                    uint8_t edidData[128];
                    if (ffReadFileData(brightness->name.chars, ARRAY_SIZE(edidData), edidData) == ARRAY_SIZE(edidData)) {
                        ffStrbufClear(&brightness->name);
                        ffEdidGetName(edidData, &brightness->name);
                    } else {
                        ffStrbufSubstrBeforeLastC(&brightness->name, '/'); // remove "/edid"
                        ffStrbufSubstrAfterLastC(&brightness->name, '/');  // try getting DRM connector name
                        if (ffCharIsDigit(brightness->name.chars[0])) {
                            // PCI address or some unknown path, give up
                            ffStrbufSetS(&brightness->name, entry->d_name);
                        } else {
                            if (ffStrbufStartsWithS(&brightness->name, "card") && ffCharIsDigit(brightness->name.chars[4])) {
                                ffStrbufSubstrAfterFirstC(&brightness->name, '-');
                            }
                        }
                    }
                } else {
                    ffStrbufInitS(&brightness->name, entry->d_name);
                }
                brightness->max = ffStrbufToDouble(&buffer, 0);
                brightness->min = 0;
                brightness->current = actualBrightness;
                brightness->builtin = true;
            }
        }
        ffStrbufSubstrBefore(&backlightDir, backlightDirLength);
    }

    return nullptr;
}

#include "detection/displayserver/displayserver.h"

#ifdef FF_HAVE_DDCUTIL
    #include "common/library.h"
    #include "common/mallocHelper.h"

    #include <ddcutil_macros.h>
    #include <ddcutil_c_api.h>

    // Try to be compatible with ddcutil 2.0
    #if DDCUTIL_VMAJOR >= 2
double ddca_set_default_sleep_multiplier(double multiplier); // ddcutil 1.4
    #else
DDCA_Status ddca_init(const char* libopts, int syslog_level, int opts);
    #endif

static const char* detectWithDdcci([[maybe_unused]] FFBrightnessOptions* options, FFlist* result) {
    FF_LIBRARY_LOAD_MESSAGE(libddcutil, "libddcutil" FF_LIBRARY_EXTENSION, 5);
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(libddcutil, ddca_get_display_info_list2)
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(libddcutil, ddca_open_display2)
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(libddcutil, ddca_get_any_vcp_value_using_explicit_type)
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(libddcutil, ddca_free_any_vcp_value)
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(libddcutil, ddca_close_display)

    // libddcutil may print trace messages to stdout during display detection, not only in ddca_init
    FF_SUPPRESS_IO();

    #ifndef FF_DISABLE_DLOPEN
    FF_LIBRARY_LOAD_SYMBOL_LAZY(libddcutil, ddca_init)
    if (ffddca_init) {
        // Ref: https://github.com/rockowitz/ddcutil/issues/344
        if (ffddca_init(nullptr, -1 /*DDCA_SYSLOG_NOT_SET*/, 1 /*DDCA_INIT_OPTIONS_DISABLE_CONFIG_FILE*/) < 0) {
            return "ddca_init() failed";
        }
    } else {
        FF_LIBRARY_LOAD_SYMBOL_LAZY(libddcutil, ddca_set_default_sleep_multiplier);
        if (ffddca_set_default_sleep_multiplier) {
            ffddca_set_default_sleep_multiplier(options->ddcciSleep / 40.0);
        }

        libddcutil = nullptr; // Don't dlclose libddcutil. See https://github.com/rockowitz/ddcutil/issues/330
    }
    #else
        #if DDCUTIL_VMAJOR >= 2
    if (ddca_init(nullptr, -1 /*DDCA_SYSLOG_NOT_SET*/, 1 /*DDCA_INIT_OPTIONS_DISABLE_CONFIG_FILE*/) < 0) {
        return "ddca_init() failed";
    }
        #else
    ddca_set_default_sleep_multiplier(options->ddcciSleep / 40.0);
        #endif
    #endif

    FF_AUTO_FREE DDCA_Display_Info_List* infoList = nullptr;
    if (ffddca_get_display_info_list2(false, &infoList) < 0) {
        return "ddca_get_display_info_list2(false, &infoList) failed";
    }

    if (infoList->ct == 0) {
        return "No DDC/CI compatible displays found";
    }

    for (int index = 0; index < infoList->ct; ++index) {
        const DDCA_Display_Info* display = &infoList->info[index];

        DDCA_Display_Handle handle;
        if (ffddca_open_display2(display->dref, false, &handle) >= 0) {
            DDCA_Any_Vcp_Value* vcpValue = nullptr;
            if (ffddca_get_any_vcp_value_using_explicit_type(handle, 0x10 /*brightness*/, DDCA_NON_TABLE_VCP_VALUE, &vcpValue) >= 0) {
                assert(vcpValue->value_type == DDCA_NON_TABLE_VCP_VALUE);
                int current = VALREC_CUR_VAL(vcpValue), max = VALREC_MAX_VAL(vcpValue);
                ffddca_free_any_vcp_value(vcpValue);

                FFBrightnessResult* brightness = FF_LIST_ADD(FFBrightnessResult, *result);
                brightness->max = max;
                brightness->min = 0;
                brightness->current = current;
                brightness->builtin = false;
                ffStrbufInitS(&brightness->name, display->model_name);
            }
            ffddca_close_display(handle);
        }
    }

    return nullptr;
}
#endif

const char* ffDetectBrightness([[maybe_unused]] FFBrightnessOptions* options, FFlist* result) {
    FF_DEBUG("Brightness: detection started (result count=%u)", result->length);
    detectWithBacklight(result);
    FF_DEBUG("Brightness: backlight detection finished (result count=%u)", result->length);

    if (options->ddcciSleep != FF_BRIGHTNESS_DDCCI_SLEEP_SKIP) {
        const FFDisplayServerResult* displayServer = ffConnectDisplayServer();
        if (result->length < displayServer->displays.length) {
            FF_DEBUG("Brightness: trying native DDC/CI (result count=%u, display count=%u)", result->length, displayServer->displays.length);
#if defined(__linux__)
            // Try native I2C first
            const char* nativeError = detectWithDdcciNative(options, result, displayServer->displays.length);
            if (nativeError != nullptr) {
#ifdef FF_HAVE_DDCUTIL
                FF_DEBUG("Native I2C DDC/CI failed: %s, falling back to ddcutil", nativeError);
                detectWithDdcci(options, result);
#else
                FF_DEBUG("Native I2C DDC/CI failed: %s; ddcutil fallback is unavailable", nativeError);
#endif
            }
#elif defined(FF_HAVE_DDCUTIL)
            detectWithDdcci(options, result);
#endif
        } else {
            FF_DEBUG("Brightness: skipping DDC/CI because result count=%u meets display count=%u", result->length, displayServer->displays.length);
        }
    } else {
        FF_DEBUG("Brightness: skipping DDC/CI because it is disabled");
    }

    FF_DEBUG("Brightness: detection finished (result count=%u)", result->length);
    return nullptr;
}
