#include "brightness.h"
#include "common/io/io.h"
#include "util/edidHelper.h"
#include "util/stringUtils.h"

#include <dirent.h>
#include <ctype.h>
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
                        if(ffStrbufStartsWithS(&brightness->name, "0000:"))
                        {
                            // PCI address, give up
                            ffStrbufSetS(&brightness->name, entry->d_name);
                        }
                        else
                        {
                            if(ffStrbufStartsWithS(&brightness->name, "card") && isdigit(brightness->name.chars[4]))
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

#include <ddcutil_c_api.h>

typedef struct DdcutilData
{
    FF_LIBRARY_SYMBOL(ddca_open_display2)
    FF_LIBRARY_SYMBOL(ddca_get_any_vcp_value_using_explicit_type)
    FF_LIBRARY_SYMBOL(ddca_free_any_vcp_value)
    FF_LIBRARY_SYMBOL(ddca_close_display)
} DdcutilData;

static void detectWithDdcciImpl(DdcutilData* data, FFlist* result, DDCA_Display_Info* dinfo)
{
    DDCA_Display_Handle handle;
    if (data->ffddca_open_display2(dinfo->dref, false, &handle) >= 0)
    {
        DDCA_Any_Vcp_Value* vcpValue = NULL;
        if (data->ffddca_get_any_vcp_value_using_explicit_type(handle, 0x10 /*brightness*/, DDCA_NON_TABLE_VCP_VALUE, &vcpValue) >= 0)
        {
            assert(vcpValue->value_type == DDCA_NON_TABLE_VCP_VALUE);
            int current = VALREC_CUR_VAL(vcpValue), max = VALREC_MAX_VAL(vcpValue);
            data->ffddca_free_any_vcp_value(vcpValue);

            FFBrightnessResult* brightness = (FFBrightnessResult*) ffListAdd(result);
            brightness->max = max;
            brightness->min = 0;
            brightness->current = current;
            ffStrbufInitS(&brightness->name, dinfo->model_name);
        }
        data->ffddca_close_display(handle);
    }
}

static const char* detectWithDdcci(FFlist* result)
{
    FF_LIBRARY_LOAD(libddcutil, &instance.config.libDdcutil, "dlopen ddcutil failed", "libddcutil" FF_LIBRARY_EXTENSION, 5)
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(libddcutil, ddca_get_display_info_list2)
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(libddcutil, ddca_get_display_info)
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(libddcutil, ddca_create_busno_display_identifier)
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(libddcutil, ddca_create_display_ref)
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(libddcutil, ddca_free_display_identifier)
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(libddcutil, ddca_free_display_ref)

    DdcutilData ddcutilData;
    FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(libddcutil, ddcutilData, ddca_open_display2)
    FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(libddcutil, ddcutilData, ddca_get_any_vcp_value_using_explicit_type)
    FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(libddcutil, ddcutilData, ddca_free_any_vcp_value)
    FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(libddcutil, ddcutilData, ddca_close_display)
    libddcutil = NULL; // Don't dlclose libddcutil. See https://github.com/rockowitz/ddcutil/issues/330

    if (instance.config.brightness.busNos.length)
    {
        FF_SUPPRESS_IO();

        FF_LIST_FOR_EACH(int, busno, instance.config.brightness.busNos)
        {
            DDCA_Display_Identifier did;
            if (ffddca_create_busno_display_identifier(*busno, &did) >= 0)
            {
                DDCA_Display_Ref dref;
                if (ffddca_create_display_ref(did, &dref) >= 0)
                {
                    FF_AUTO_FREE DDCA_Display_Info* dinfo = NULL;
                    if (ffddca_get_display_info(dref, &dinfo) >= 0)
                        detectWithDdcciImpl(&ddcutilData, result, dinfo);
                    ffddca_free_display_ref(dref);
                }

                ffddca_free_display_identifier(did);
            }
        }

        return NULL;
    }

    FF_AUTO_FREE DDCA_Display_Info_List* infoList = NULL;
    if (ffddca_get_display_info_list2(false, &infoList) < 0)
        return "ddca_get_display_info_list2(false, &infoList) failed";

    if (infoList->ct == 0)
        return "No DDC/CI compatible displays found";

    for (int index = 0; index < infoList->ct; ++index)
    {
        detectWithDdcciImpl(&ddcutilData, result, &infoList->info[index]);
    }

    return NULL;
}
#endif

const char* ffDetectBrightness(FFlist* result)
{
    detectWithBacklight(result);

    #ifdef FF_HAVE_DDCUTIL
    const FFDisplayServerResult* displayServer = ffConnectDisplayServer();
    if (result->length < displayServer->displays.length)
        detectWithDdcci(result);
    #endif

    return NULL;
}
