#include "brightness.h"
#include "common/io/io.h"
#include "util/edidHelper.h"
#include "util/stringUtils.h"

#include <dirent.h>
#include <limits.h>

static const char* detectWithBacklight(FFlist* result)
{
    //https://www.kernel.org/doc/Documentation/ABI/stable/sysfs-class-backlight
    const char* backlightDirPath = "/sys/class/backlight/";

    DIR* dirp = opendir(backlightDirPath);
    if(dirp == NULL)
        return "Failed to open `/sys/class/backlight/`";

    FF_STRBUF_AUTO_DESTROY backlightDir = ffStrbufCreateA(64);
    ffStrbufAppendS(&backlightDir, backlightDirPath);

    uint32_t backlightDirLength = backlightDir.length;

    FF_STRBUF_AUTO_DESTROY buffer = ffStrbufCreate();

    struct dirent* entry;
    while((entry = readdir(dirp)) != NULL)
    {
        if(ffStrEquals(entry->d_name, ".") || ffStrEquals(entry->d_name, ".."))
            continue;

        ffStrbufAppendS(&backlightDir, entry->d_name);
        ffStrbufAppendS(&backlightDir, "/actual_brightness");
        if(ffReadFileBuffer(backlightDir.chars, &buffer))
        {
            double actualBrightness = ffStrbufToDouble(&buffer);
            ffStrbufSubstrBefore(&backlightDir, backlightDirLength);
            ffStrbufAppendS(&backlightDir, entry->d_name);
            ffStrbufAppendS(&backlightDir, "/max_brightness");
            if(ffReadFileBuffer(backlightDir.chars, &buffer))
            {
                FFBrightnessResult* brightness = (FFBrightnessResult*) ffListAdd(result);
                ffStrbufSubstrBeforeLastC(&backlightDir, '/');
                ffStrbufAppendS(&backlightDir, "/device");
                ffStrbufInitA(&brightness->name, PATH_MAX + 1);
                if(realpath(backlightDir.chars, brightness->name.chars))
                {
                    ffStrbufRecalculateLength(&brightness->name);
                    // if we managed to get edid, use it
                    ffStrbufAppendS(&brightness->name, "/edid");
                    uint8_t edidData[128];
                    if(ffReadFileData(brightness->name.chars, sizeof(edidData), edidData) == sizeof(edidData))
                    {
                        ffStrbufClear(&brightness->name);
                        ffEdidGetName(edidData, &brightness->name);
                    }
                    else
                    {
                        ffStrbufSubstrBeforeLastC(&brightness->name, '/'); // remove "/edid"
                        ffStrbufSubstrAfterLastC(&brightness->name, '/'); // try getting DRM connector name
                        if(ffCharIsDigit(brightness->name.chars[0]))
                        {
                            // PCI address or some unknown path, give up
                            ffStrbufSetS(&brightness->name, entry->d_name);
                        }
                        else
                        {
                            if(ffStrbufStartsWithS(&brightness->name, "card") && ffCharIsDigit(brightness->name.chars[4]))
                                ffStrbufSubstrAfterFirstC(&brightness->name, '-');
                        }
                    }
                }
                else
                    ffStrbufInitS(&brightness->name, entry->d_name);
                brightness->max = ffStrbufToDouble(&buffer);
                brightness->min = 0;
                brightness->current = actualBrightness;
            }
        }
        ffStrbufSubstrBefore(&backlightDir, backlightDirLength);
    }

    closedir(dirp);

    return NULL;
}

#ifdef FF_HAVE_DDCUTIL
#include "detection/displayserver/displayserver.h"
#include "common/library.h"
#include "util/mallocHelper.h"

#include <ddcutil_macros.h>
#include <ddcutil_c_api.h>

// Try to be compatible with ddcutil 2.0
#if DDCUTIL_VMAJOR >= 2
double ddca_set_default_sleep_multiplier(double multiplier); // ddcutil 1.4
#else
DDCA_Status ddca_init(const char *libopts, int syslog_level, int opts);
#endif

static const char* detectWithDdcci(FFBrightnessOptions* options, FFlist* result)
{
    FF_LIBRARY_LOAD(libddcutil, &instance.config.library.libDdcutil, "dlopen ddcutil failed", "libddcutil" FF_LIBRARY_EXTENSION, 5);
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(libddcutil, ddca_get_display_info_list2)
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(libddcutil, ddca_open_display2)
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(libddcutil, ddca_get_any_vcp_value_using_explicit_type)
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(libddcutil, ddca_free_any_vcp_value)
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(libddcutil, ddca_close_display)

    #ifndef FF_DISABLE_DLOPEN
    __typeof__(&ddca_init) ffddca_init = dlsym(libddcutil, "ddca_init");
    if (ffddca_init)
    #else
    __typeof__(&ddca_init) ffddca_init = ddca_init;
    #endif
    {
        FF_SUPPRESS_IO();
        // Ref: https://github.com/rockowitz/ddcutil/issues/344
        if (ffddca_init(NULL, -1 /*DDCA_SYSLOG_NOT_SET*/, 1 /*DDCA_INIT_OPTIONS_DISABLE_CONFIG_FILE*/) < 0)
            return "ddca_init() failed";
    }
    #ifndef FF_DISABLE_DLOPEN
    else
    {
        __typeof__(&ddca_set_default_sleep_multiplier) ffddca_set_default_sleep_multiplier = dlsym(libddcutil, "ddca_set_default_sleep_multiplier");
        if (ffddca_set_default_sleep_multiplier)
            ffddca_set_default_sleep_multiplier(options->ddcciSleep / 40.0);

        libddcutil = NULL; // Don't dlclose libddcutil. See https://github.com/rockowitz/ddcutil/issues/330
    }
    #endif

    FF_AUTO_FREE DDCA_Display_Info_List* infoList = NULL;
    if (ffddca_get_display_info_list2(false, &infoList) < 0)
        return "ddca_get_display_info_list2(false, &infoList) failed";

    if (infoList->ct == 0)
        return "No DDC/CI compatible displays found";

    for (int index = 0; index < infoList->ct; ++index)
    {
        const DDCA_Display_Info* display = &infoList->info[index];

        DDCA_Display_Handle handle;
        if (ffddca_open_display2(display->dref, false, &handle) >= 0)
        {
            DDCA_Any_Vcp_Value* vcpValue = NULL;
            if (ffddca_get_any_vcp_value_using_explicit_type(handle, 0x10 /*brightness*/, DDCA_NON_TABLE_VCP_VALUE, &vcpValue) >= 0)
            {
                assert(vcpValue->value_type == DDCA_NON_TABLE_VCP_VALUE);
                int current = VALREC_CUR_VAL(vcpValue), max = VALREC_MAX_VAL(vcpValue);
                ffddca_free_any_vcp_value(vcpValue);

                FFBrightnessResult* brightness = (FFBrightnessResult*) ffListAdd(result);
                brightness->max = max;
                brightness->min = 0;
                brightness->current = current;
                ffStrbufInitS(&brightness->name, display->model_name);
            }
            ffddca_close_display(handle);
        }
    }

    return NULL;
}
#endif

const char* ffDetectBrightness(FF_MAYBE_UNUSED FFBrightnessOptions* options, FFlist* result)
{
    detectWithBacklight(result);

    #ifdef FF_HAVE_DDCUTIL
    const FFDisplayServerResult* displayServer = ffConnectDisplayServer();
    if (result->length < displayServer->displays.length)
        detectWithDdcci(options, result);
    #endif

    return NULL;
}
