#include "brightness.h"
#include "common/arrayUtils.h"
#include "common/io.h"
#include "common/kmod.h"
#include "common/debug.h"
#include "common/time.h"

#include <sys/ioctl.h>
#include <sys/fcntl.h>
#include <unistd.h>

#include <dev/iicbus/iic.h>

const char* detectWithDdcci(FF_A_UNUSED FFBrightnessOptions* options, FFlist* result) {
    // FIXME: doesn't work for me
    for (char i = '0'; i <= '9'; ++i) {
        char path[] = "/dev/iic0";
        path[ARRAY_SIZE(path) - 2] = i;

        FF_AUTO_CLOSE_FD int fd = open(path, O_RDWR | O_CLOEXEC);
        if (fd < 0) {
            int err = errno;
            if (err == ENOENT) {
                FF_DEBUG("No more I2C devices");
                break;
            } else if (i == '0' && !ffKmodLoaded("iicbus")) {
                FF_DEBUG("iicbus module is not loaded");
                return "No I2C devices found and iicbus module is not loaded";
            }
            FF_DEBUG("open(/dev/iic%c) failed: %s", i, strerror(errno));
            continue;
        }

        uint8_t i2cIn[] = { FF_DDC_CI_VCP_COMMAND, FF_DDC_CI_MAKE_HEADER(2), FF_DDC_CI_GET_VCP, FF_DDC_CI_LUMINANCE_OPCODE, 0 };
        i2cIn[4] = FF_DDC_CI_WRITE_ADDR ^ i2cIn[0] ^ i2cIn[1] ^ i2cIn[2] ^ i2cIn[3];

        int ret = ioctl(fd, I2CRDWR, &(struct iic_rdwr_data) {
            .msgs = &(struct iic_msg) {
                .slave = FF_DDC_CI_WRITE_ADDR,
                .flags = IIC_M_WR,
                .len = ARRAY_SIZE(i2cIn),
                .buf = i2cIn
            },
            .nmsgs = 1
        });
        if (ret < 0) {
            FF_DEBUG("First ioctl(/dev/iic%c, I2CRDWR) failed: %s", i, strerror(errno));
            continue;
        }

        ffTimeSleep(options->ddcciSleep);

        uint8_t i2cOut[12] = {};
        ret = ioctl(fd, I2CRDWR, &(struct iic_rdwr_data) {
            .msgs = &(struct iic_msg) {
                .slave = FF_DDC_CI_READ_ADDR, // LSB will be overridden by kernel to set read bit
                .flags = IIC_M_RD,
                .len = ARRAY_SIZE(i2cOut),
                .buf = i2cOut
            },
            .nmsgs = 1
        });
        if (ret < 0) {
            FF_DEBUG("Second ioctl(/dev/iic%c, I2CRDWR) failed: %s", i, strerror(errno));
            continue;
        }
        if (i2cOut[2] != 0x02 || i2cOut[3] != 0x00) {
            FF_DEBUG("i2c out validation failed: i2cOut[2] = 0x%02x, i2cOut[3] = 0x%02x", i2cOut[2], i2cOut[3]);
            continue;
        }

        uint32_t current = ((uint32_t) i2cOut[8] << 8u) + (uint32_t) i2cOut[9];
        uint32_t max = ((uint32_t) i2cOut[6] << 8u) + (uint32_t) i2cOut[7];

        FFBrightnessResult* brightness = FF_LIST_ADD(FFBrightnessResult, *result);
        brightness->max = max;
        brightness->min = 0;
        brightness->current = current;
        ffStrbufInitS(&brightness->name, path + strlen("/dev/"));
        brightness->builtin = false;
    }

    return NULL;
}

#if __has_include(<sys/backlight.h>)

    #include <sys/backlight.h>


const char* detectWithBacklight(FF_A_UNUSED FFBrightnessOptions* options, FFlist* result) {
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
    return NULL;
}

#else

const char* detectWithBacklight(FF_A_UNUSED FFBrightnessOptions* options, FF_A_UNUSED FFlist* result) {
    FF_DEBUG("Backlight support is not available on this system");
    return "Backlight is supported only on FreeBSD 13 and newer";
}

#endif


const char* ffDetectBrightness(FF_A_UNUSED FFBrightnessOptions* options, FFlist* result) {
    detectWithBacklight(options, result);
    if (result->length == 0) {
        detectWithDdcci(options, result);
    }
    return NULL;
}
