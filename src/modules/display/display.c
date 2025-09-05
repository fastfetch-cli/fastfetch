#include "common/printing.h"
#include "common/jsonconfig.h"
#include "common/size.h"
#include "detection/displayserver/displayserver.h"
#include "modules/display/display.h"
#include "util/stringUtils.h"

#include <math.h>

static int sortByNameAsc(FFDisplayResult* a, FFDisplayResult* b)
{
    return ffStrbufComp(&a->name, &b->name);
}

static int sortByNameDesc(FFDisplayResult* a, FFDisplayResult* b)
{
    return -ffStrbufComp(&a->name, &b->name);
}

bool ffPrintDisplay(FFDisplayOptions* options)
{
    const FFDisplayServerResult* dsResult = ffConnectDisplayServer();

    if(dsResult->displays.length == 0)
    {
        ffPrintError(FF_DISPLAY_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "Couldn't detect display");
        return false;
    }

    if (options->order != FF_DISPLAY_ORDER_NONE)
    {
        ffListSort((FFlist*) &dsResult->displays, (void*) (options->order == FF_DISPLAY_ORDER_ASC ? sortByNameAsc : sortByNameDesc));
    }

    if (options->compactType != FF_DISPLAY_COMPACT_TYPE_NONE)
    {
        ffPrintLogoAndKey(FF_DISPLAY_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT);

        FF_STRBUF_AUTO_DESTROY buffer = ffStrbufCreate();
        FF_LIST_FOR_EACH(FFDisplayResult, result, dsResult->displays)
        {
            if (options->compactType & FF_DISPLAY_COMPACT_TYPE_ORIGINAL_BIT)
            {
                ffStrbufAppendF(&buffer, "%ix%i", result->width, result->height);
            }
            else
            {
                ffStrbufAppendF(&buffer, "%ix%i", result->scaledWidth, result->scaledHeight);
            }

            if (options->compactType & FF_DISPLAY_COMPACT_TYPE_REFRESH_RATE_BIT)
            {
                if (result->refreshRate > 0)
                {
                    const char* space = instance.config.display.freqSpaceBeforeUnit == FF_SPACE_BEFORE_UNIT_ALWAYS ? " " : "";
                    if (options->preciseRefreshRate)
                        ffStrbufAppendF(&buffer, " @ %g%sHz", result->refreshRate, space);
                    else
                        ffStrbufAppendF(&buffer, " @ %i%sHz", (uint32_t) (result->refreshRate + 0.5), space);
                }
                ffStrbufAppendS(&buffer, ", ");
            }
            else
            {
                ffStrbufAppendC(&buffer, ' ');
            }
        }
        ffStrbufTrimRight(&buffer, ' ');
        ffStrbufTrimRight(&buffer, ',');
        ffStrbufPutTo(&buffer, stdout);
        return true;
    }

    FF_STRBUF_AUTO_DESTROY key = ffStrbufCreate();

    for(uint32_t i = 0; i < dsResult->displays.length; i++)
    {
        FFDisplayResult* result = FF_LIST_GET(FFDisplayResult, dsResult->displays, i);
        uint32_t moduleIndex = dsResult->displays.length == 1 ? 0 : i + 1;
        const char* displayType = result->type == FF_DISPLAY_TYPE_UNKNOWN ? NULL : result->type == FF_DISPLAY_TYPE_BUILTIN ? "Built-in" : "External";

        ffStrbufClear(&key);
        if(options->moduleArgs.key.length == 0)
        {
            if (result->name.length)
                ffStrbufAppendF(&key, "%s (%s)", FF_DISPLAY_MODULE_NAME, result->name.chars);
            else if (moduleIndex > 0)
                ffStrbufAppendF(&key, "%s (%d)", FF_DISPLAY_MODULE_NAME, moduleIndex);
            else
                ffStrbufAppendS(&key, FF_DISPLAY_MODULE_NAME);
        }
        else
        {
            FF_PARSE_FORMAT_STRING_CHECKED(&key, &options->moduleArgs.key, ((FFformatarg[]) {
                FF_FORMAT_ARG(moduleIndex, "index"),
                FF_FORMAT_ARG(result->name, "name"),
                FF_FORMAT_ARG(displayType, "type"),
                FF_FORMAT_ARG(options->moduleArgs.keyIcon, "icon"),
            }));
        }

        FF_STRBUF_AUTO_DESTROY buffer = ffStrbufCreate();
        double inch = sqrt(result->physicalWidth * result->physicalWidth + result->physicalHeight * result->physicalHeight) / 25.4;
        double scaleFactor = (double) result->height / (double) result->scaledHeight;

        if(options->moduleArgs.outputFormat.length == 0)
        {
            ffPrintLogoAndKey(key.chars, 0, &options->moduleArgs, FF_PRINT_TYPE_NO_CUSTOM_KEY);

            ffStrbufAppendF(&buffer, "%ix%i", result->width, result->height);

            if(
                result->scaledWidth > 0 && result->scaledWidth != result->width &&
                result->scaledHeight > 0 && result->scaledHeight != result->height)
            {
                ffStrbufAppendS(&buffer, " @ ");
                ffStrbufAppendDouble(&buffer, scaleFactor, instance.config.display.fractionNdigits, instance.config.display.fractionTrailingZeros == FF_FRACTION_TRAILING_ZEROS_TYPE_ALWAYS);
                ffStrbufAppendC(&buffer, 'x');
            }

            if (inch > 1)
                ffStrbufAppendF(&buffer, " in %i\"", (uint32_t) (inch + 0.5));

            if(result->refreshRate > 0)
            {
                ffStrbufAppendS(&buffer, ", ");
                if(options->preciseRefreshRate)
                    ffStrbufAppendDouble(&buffer, result->refreshRate, 3, false);
                else
                    ffStrbufAppendSInt(&buffer, (int) (result->refreshRate + 0.5));
                ffStrbufAppendS(&buffer, instance.config.display.freqSpaceBeforeUnit == FF_SPACE_BEFORE_UNIT_NEVER ? "Hz" : " Hz");
            }

            bool flag = false;
            if (result->type != FF_DISPLAY_TYPE_UNKNOWN)
            {
                ffStrbufAppendS(&buffer, result->type == FF_DISPLAY_TYPE_BUILTIN ? " [Built-in" : " [External");
                flag = true;
            }

            if (result->hdrStatus == FF_DISPLAY_HDR_STATUS_ENABLED)
            {
                ffStrbufAppendS(&buffer, flag ? ", HDR" : " [HDR");
                flag = true;
            }

            if (flag)
                ffStrbufAppendS(&buffer, "]");

            if(moduleIndex > 0 && result->primary)
                ffStrbufAppendS(&buffer, " *");

            ffStrbufPutTo(&buffer, stdout);
            ffStrbufClear(&buffer);
        }
        else
        {
            double ppi = inch == 0 ? 0 : sqrt(result->width * result->width + result->height * result->height) / inch;
            bool hdrEnabled = result->hdrStatus == FF_DISPLAY_HDR_STATUS_ENABLED;
            bool hdrCompatible = result->hdrStatus == FF_DISPLAY_HDR_STATUS_SUPPORTED || result->hdrStatus == FF_DISPLAY_HDR_STATUS_ENABLED;
            uint32_t iInch = (uint32_t) (inch + 0.5), iPpi = (uint32_t) (ppi + 0.5);

            char refreshRate[16];
            if(result->refreshRate > 0)
            {
                if(options->preciseRefreshRate)
                    snprintf(refreshRate, ARRAY_SIZE(refreshRate), "%g", ((int) (result->refreshRate * 1000 + 0.5)) / 1000.0);
                else
                    snprintf(refreshRate, ARRAY_SIZE(refreshRate), "%i", (uint32_t) (result->refreshRate + 0.5));
            }
            else
                refreshRate[0] = 0;

            char preferredRefreshRate[16];
            if(result->preferredRefreshRate > 0)
            {
                if(options->preciseRefreshRate)
                    snprintf(preferredRefreshRate, ARRAY_SIZE(preferredRefreshRate), "%g", ((int) (result->preferredRefreshRate * 1000 + 0.5)) / 1000.0);
                else
                    snprintf(preferredRefreshRate, ARRAY_SIZE(preferredRefreshRate), "%i", (uint32_t) (result->preferredRefreshRate + 0.5));
            }
            else
                preferredRefreshRate[0] = 0;

            char buf[32];
            if (result->serial)
            {
                const uint8_t* nums = (uint8_t*) &result->serial;
                snprintf(buf, ARRAY_SIZE(buf), "%2X-%2X-%2X-%2X", nums[0], nums[1], nums[2], nums[3]);
            }
            else
                buf[0] = '\0';

            FF_PRINT_FORMAT_CHECKED(key.chars, 0, &options->moduleArgs, FF_PRINT_TYPE_NO_CUSTOM_KEY, ((FFformatarg[]) {
                FF_FORMAT_ARG(result->width, "width"),
                FF_FORMAT_ARG(result->height, "height"),
                FF_FORMAT_ARG(refreshRate, "refresh-rate"),
                FF_FORMAT_ARG(result->scaledWidth, "scaled-width"),
                FF_FORMAT_ARG(result->scaledHeight, "scaled-height"),
                FF_FORMAT_ARG(result->name, "name"),
                FF_FORMAT_ARG(displayType, "type"),
                FF_FORMAT_ARG(result->rotation, "rotation"),
                FF_FORMAT_ARG(result->primary, "is-primary"),
                FF_FORMAT_ARG(result->physicalWidth, "physical-width"),
                FF_FORMAT_ARG(result->physicalHeight, "physical-height"),
                FF_FORMAT_ARG(iInch, "inch"),
                FF_FORMAT_ARG(iPpi, "ppi"),
                FF_FORMAT_ARG(result->bitDepth, "bit-depth"),
                FF_FORMAT_ARG(hdrEnabled, "hdr-enabled"),
                FF_FORMAT_ARG(result->manufactureYear, "manufacture-year"),
                FF_FORMAT_ARG(result->manufactureWeek, "manufacture-week"),
                FF_FORMAT_ARG(buf, "serial"),
                FF_FORMAT_ARG(result->platformApi, "platform-api"),
                FF_FORMAT_ARG(hdrCompatible, "hdr-compatible"),
                FF_FORMAT_ARG(scaleFactor, "scale-factor"),
                FF_FORMAT_ARG(result->preferredWidth, "preferred-width"),
                FF_FORMAT_ARG(result->preferredHeight, "preferred-height"),
                FF_FORMAT_ARG(preferredRefreshRate, "preferred-refresh-rate"),
            }));
        }
    }

    return true;
}

void ffParseDisplayJsonObject(FFDisplayOptions* options, yyjson_val* module)
{
    yyjson_val *key, *val;
    size_t idx, max;
    yyjson_obj_foreach(module, idx, max, key, val)
    {
        if (ffJsonConfigParseModuleArgs(key, val, &options->moduleArgs))
            continue;

        if (unsafe_yyjson_equals_str(key, "compactType"))
        {
            if (yyjson_is_null(val))
                options->compactType = FF_DISPLAY_COMPACT_TYPE_NONE;
            else
            {
                int value;
                const char* error = ffJsonConfigParseEnum(val, &value, (FFKeyValuePair[]) {
                    { "none", FF_DISPLAY_COMPACT_TYPE_NONE },
                    { "original", FF_DISPLAY_COMPACT_TYPE_ORIGINAL_BIT },
                    { "scaled", FF_DISPLAY_COMPACT_TYPE_SCALED_BIT },
                    { "original-with-refresh-rate", FF_DISPLAY_COMPACT_TYPE_ORIGINAL_BIT | FF_DISPLAY_COMPACT_TYPE_REFRESH_RATE_BIT },
                    { "scaled-with-refresh-rate", FF_DISPLAY_COMPACT_TYPE_SCALED_BIT | FF_DISPLAY_COMPACT_TYPE_REFRESH_RATE_BIT },
                    {},
                });
                if (error)
                    ffPrintError(FF_DISPLAY_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "Invalid %s value: %s", unsafe_yyjson_get_str(key), error);
                else
                    options->compactType = (FFDisplayCompactType) value;
            }
            continue;
        }

        if (unsafe_yyjson_equals_str(key, "preciseRefreshRate"))
        {
            options->preciseRefreshRate = yyjson_get_bool(val);
            continue;
        }

        if (unsafe_yyjson_equals_str(key, "order"))
        {
            if (yyjson_is_null(val))
                options->order = FF_DISPLAY_ORDER_NONE;
            else
            {
                int value;
                const char* error = ffJsonConfigParseEnum(val, &value, (FFKeyValuePair[]) {
                    { "asc", FF_DISPLAY_ORDER_ASC },
                    { "desc", FF_DISPLAY_ORDER_DESC },
                    { "none", FF_DISPLAY_ORDER_NONE },
                    {},
                });
                if (error)
                    ffPrintError(FF_DISPLAY_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "Invalid %s value: %s", unsafe_yyjson_get_str(key), error);
                else
                    options->order = (FFDisplayOrder) value;
            }
            continue;
        }

        ffPrintError(FF_DISPLAY_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "Unknown JSON key %s", unsafe_yyjson_get_str(key));
    }
}

void ffGenerateDisplayJsonConfig(FFDisplayOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    ffJsonConfigGenerateModuleArgsConfig(doc, module, &options->moduleArgs);

    switch ((int) options->compactType)
    {
        case FF_DISPLAY_COMPACT_TYPE_NONE:
            yyjson_mut_obj_add_str(doc, module, "compactType", "none");
            break;
        case FF_DISPLAY_COMPACT_TYPE_ORIGINAL_BIT:
            yyjson_mut_obj_add_str(doc, module, "compactType", "original");
            break;
        case FF_DISPLAY_COMPACT_TYPE_SCALED_BIT:
            yyjson_mut_obj_add_str(doc, module, "compactType", "scaled");
            break;
        case FF_DISPLAY_COMPACT_TYPE_ORIGINAL_BIT | FF_DISPLAY_COMPACT_TYPE_REFRESH_RATE_BIT:
            yyjson_mut_obj_add_str(doc, module, "compactType", "original-with-refresh-rate");
            break;
        case FF_DISPLAY_COMPACT_TYPE_SCALED_BIT | FF_DISPLAY_COMPACT_TYPE_REFRESH_RATE_BIT:
            yyjson_mut_obj_add_str(doc, module, "compactType", "scaled-with-refresh-rate");
            break;
    }

    yyjson_mut_obj_add_bool(doc, module, "preciseRefreshRate", options->preciseRefreshRate);

    switch (options->order)
    {
        case FF_DISPLAY_ORDER_NONE:
            yyjson_mut_obj_add_null(doc, module, "order");
            break;
        case FF_DISPLAY_ORDER_ASC:
            yyjson_mut_obj_add_str(doc, module, "order", "asc");
            break;
        case FF_DISPLAY_ORDER_DESC:
            yyjson_mut_obj_add_str(doc, module, "order", "desc");
            break;
    }
}

bool ffGenerateDisplayJsonResult(FF_MAYBE_UNUSED FFDisplayOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    const FFDisplayServerResult* dsResult = ffConnectDisplayServer();

    if(dsResult->displays.length == 0)
    {
        yyjson_mut_obj_add_str(doc, module, "error", "Couldn't detect display");
        return false;
    }

    yyjson_mut_val* arr = yyjson_mut_obj_add_arr(doc, module, "result");
    FF_LIST_FOR_EACH(FFDisplayResult, item, dsResult->displays)
    {
        yyjson_mut_val* obj = yyjson_mut_arr_add_obj(doc, arr);
        yyjson_mut_obj_add_uint(doc, obj, "id", item->id);
        yyjson_mut_obj_add_strbuf(doc, obj, "name", &item->name);
        yyjson_mut_obj_add_bool(doc, obj, "primary", item->primary);

        yyjson_mut_val* output = yyjson_mut_obj_add_obj(doc, obj, "output");
        yyjson_mut_obj_add_uint(doc, output, "width", item->width);
        yyjson_mut_obj_add_uint(doc, output, "height", item->height);
        yyjson_mut_obj_add_real(doc, output, "refreshRate", item->refreshRate);

        yyjson_mut_val* scaled = yyjson_mut_obj_add_obj(doc, obj, "scaled");
        yyjson_mut_obj_add_uint(doc, scaled, "width", item->scaledWidth);
        yyjson_mut_obj_add_uint(doc, scaled, "height", item->scaledHeight);

        yyjson_mut_val* preferred = yyjson_mut_obj_add_obj(doc, obj, "preferred");
        yyjson_mut_obj_add_uint(doc, preferred, "width", item->preferredWidth);
        yyjson_mut_obj_add_uint(doc, preferred, "height", item->preferredHeight);
        yyjson_mut_obj_add_real(doc, preferred, "refreshRate", item->preferredRefreshRate);

        yyjson_mut_val* physical = yyjson_mut_obj_add_obj(doc, obj, "physical");
        yyjson_mut_obj_add_uint(doc, physical, "width", item->physicalWidth);
        yyjson_mut_obj_add_uint(doc, physical, "height", item->physicalHeight);

        yyjson_mut_obj_add_uint(doc, obj, "rotation", item->rotation);
        yyjson_mut_obj_add_uint(doc, obj, "bitDepth", item->bitDepth);
        if (item->hdrStatus == FF_DISPLAY_HDR_STATUS_UNKNOWN)
            yyjson_mut_obj_add_null(doc, obj, "hdrStatus");
        else switch (item->hdrStatus)
        {
            case FF_DISPLAY_HDR_STATUS_UNSUPPORTED:
                yyjson_mut_obj_add_str(doc, obj, "hdrStatus", "Unsupported");
                break;
            case FF_DISPLAY_HDR_STATUS_SUPPORTED:
                yyjson_mut_obj_add_str(doc, obj, "hdrStatus", "Supported");
                break;
            case FF_DISPLAY_HDR_STATUS_ENABLED:
                yyjson_mut_obj_add_str(doc, obj, "hdrStatus", "Enabled");
                break;
            default:
                yyjson_mut_obj_add_str(doc, obj, "hdrStatus", "Unknown");
                break;
        }

        switch (item->type)
        {
            case FF_DISPLAY_TYPE_BUILTIN:
                yyjson_mut_obj_add_str(doc, obj, "type", "Builtin");
                break;
            case FF_DISPLAY_TYPE_EXTERNAL:
                yyjson_mut_obj_add_str(doc, obj, "type", "External");
                break;
            default:
                yyjson_mut_obj_add_str(doc, obj, "type", "Unknown");
                break;
        }

        if (item->manufactureYear)
        {
            yyjson_mut_val* manufactureDate = yyjson_mut_obj_add_obj(doc, obj, "manufactureDate");
            yyjson_mut_obj_add_uint(doc, manufactureDate, "year", item->manufactureYear);
            yyjson_mut_obj_add_uint(doc, manufactureDate, "week", item->manufactureWeek);
        }
        else
        {
            yyjson_mut_obj_add_null(doc, obj, "manufactureDate");
        }

        if (item->serial)
            yyjson_mut_obj_add_uint(doc, obj, "serial", item->serial);
        else
            yyjson_mut_obj_add_null(doc, obj, "serial");

        yyjson_mut_obj_add_str(doc, obj, "platformApi", item->platformApi);
    }

    return true;
}

void ffInitDisplayOptions(FFDisplayOptions* options)
{
    ffOptionInitModuleArg(&options->moduleArgs, "󰍹");
    options->compactType = FF_DISPLAY_COMPACT_TYPE_NONE;
    options->preciseRefreshRate = false;
}

void ffDestroyDisplayOptions(FFDisplayOptions* options)
{
    ffOptionDestroyModuleArg(&options->moduleArgs);
}

FFModuleBaseInfo ffDisplayModuleInfo = {
    .name = FF_DISPLAY_MODULE_NAME,
    .description = "Print resolutions, refresh rates, etc",
    .initOptions = (void*) ffInitDisplayOptions,
    .destroyOptions = (void*) ffDestroyDisplayOptions,
    .parseJsonObject = (void*) ffParseDisplayJsonObject,
    .printModule = (void*) ffPrintDisplay,
    .generateJsonResult = (void*) ffGenerateDisplayJsonResult,
    .generateJsonConfig = (void*) ffGenerateDisplayJsonConfig,
    .formatArgs = FF_FORMAT_ARG_LIST(((FFModuleFormatArg[]) {
        {"Screen configured width (in pixels)", "width"},
        {"Screen configured height (in pixels)", "height"},
        {"Screen configured refresh rate (in Hz)", "refresh-rate"},
        {"Screen scaled width (in pixels)", "scaled-width"},
        {"Screen scaled height (in pixels)", "scaled-height"},
        {"Screen name", "name"},
        {"Screen type (Built-in or External)", "type"},
        {"Screen rotation (in degrees)", "rotation"},
        {"True if being the primary screen", "is-primary"},
        {"Screen physical width (in millimeters)", "physical-width"},
        {"Screen physical height (in millimeters)", "physical-height"},
        {"Physical diagonal length in inches", "inch"},
        {"Pixels per inch (PPI)", "ppi"},
        {"Bits per color channel", "bit-depth"},
        {"True if high dynamic range (HDR) mode is enabled", "hdr-enabled"},
        {"Year of manufacturing", "manufacture-year"},
        {"Nth week of manufacturing in the year", "manufacture-week"},
        {"Serial number", "serial"},
        {"The platform API used when detecting the display", "platform-api"},
        {"True if the display is HDR compatible", "hdr-compatible"},
        {"HiDPI scale factor", "scale-factor"},
        {"Screen preferred width (in pixels)", "preferred-width"},
        {"Screen preferred height (in pixels)", "preferred-height"},
        {"Screen preferred refresh rate (in Hz)", "preferred-refresh-rate"},
    }))
};
