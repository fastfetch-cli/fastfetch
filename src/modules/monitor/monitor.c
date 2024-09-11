#include "common/printing.h"
#include "common/jsonconfig.h"
#include "detection/monitor/monitor.h"
#include "modules/monitor/monitor.h"
#include "util/stringUtils.h"

#include <math.h>

#define FF_MONITOR_NUM_FORMAT_ARGS 11

void ffPrintMonitor(FFMonitorOptions* options)
{
    FF_LIST_AUTO_DESTROY result = ffListCreate(sizeof(FFMonitorResult));

    const char* error = ffDetectMonitor(&result);

    if(error)
    {
        ffPrintError(FF_MONITOR_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "%s", error);
        return;
    }

    if(!result.length)
    {
        ffPrintError(FF_MONITOR_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "No physical display detected");
        return;
    }

    FF_STRBUF_AUTO_DESTROY key = ffStrbufCreate();
    uint32_t index = 0;
    FF_LIST_FOR_EACH(FFMonitorResult, display, result)
    {
        double inch = sqrt(display->physicalWidth * display->physicalWidth + display->physicalHeight * display->physicalHeight) / 25.4;
        double ppi = sqrt(display->width * display->width + display->height * display->height) / inch;

        ffStrbufClear(&key);
        if(options->moduleArgs.key.length == 0)
        {
            ffStrbufAppendF(&key, "%s (%s)", FF_MONITOR_MODULE_NAME, display->name.chars);
        }
        else
        {
            uint32_t moduleIndex = result.length == 1 ? 0 : index + 1;
            FF_PARSE_FORMAT_STRING_CHECKED(&key, &options->moduleArgs.key, 3, ((FFformatarg[]){
                FF_FORMAT_ARG(moduleIndex, "index"),
                FF_FORMAT_ARG(display->name, "name"),
                FF_FORMAT_ARG(options->moduleArgs.keyIcon, "icon"),
            }));
        }

        if(options->moduleArgs.outputFormat.length == 0)
        {
            ffPrintLogoAndKey(key.chars, 0, &options->moduleArgs, FF_PRINT_TYPE_NO_CUSTOM_KEY);

            printf("%ux%u px", display->width, display->height);
            if (display->refreshRate > 0)
                printf(" @ %g Hz", ((int) (display->refreshRate * 1000 + 0.5)) / 1000.0);
            if (inch > 0)
                printf(" - %ux%u mm (%.2f inches, %.2f ppi)\n", display->physicalWidth, display->physicalHeight, inch, ppi);
            else
                putchar('\n');
        }
        else
        {
            char buf[32];
            if (display->serial)
            {
                const uint8_t* nums = (uint8_t*) &display->serial;
                snprintf(buf, sizeof(buf), "%2X-%2X-%2X-%2X", nums[0], nums[1], nums[2], nums[3]);
            }
            else
                buf[0] = '\0';

            FF_PRINT_FORMAT_CHECKED(key.chars, 0, &options->moduleArgs, FF_PRINT_TYPE_NO_CUSTOM_KEY, FF_MONITOR_NUM_FORMAT_ARGS, ((FFformatarg[]) {
                FF_FORMAT_ARG(display->name, "name"),
                FF_FORMAT_ARG(display->width, "width"),
                FF_FORMAT_ARG(display->height, "height"),
                FF_FORMAT_ARG(display->physicalWidth, "physical-width"),
                FF_FORMAT_ARG(display->physicalHeight, "physical-height"),
                FF_FORMAT_ARG(inch, "inch"),
                FF_FORMAT_ARG(ppi, "ppi"),
                FF_FORMAT_ARG(display->manufactureYear, "manufacture-year"),
                FF_FORMAT_ARG(display->manufactureWeek, "manufacture-week"),
                FF_FORMAT_ARG(buf, "serial"),
                FF_FORMAT_ARG(display->refreshRate, "refresh-rate"),
            }));
        }

        ffStrbufDestroy(&display->name);
        ++index;
    }
}

bool ffParseMonitorCommandOptions(FFMonitorOptions* options, const char* key, const char* value)
{
    const char* subKey = ffOptionTestPrefix(key, FF_MONITOR_MODULE_NAME);
    if (!subKey) return false;
    if (ffOptionParseModuleArgs(key, subKey, value, &options->moduleArgs))
        return true;

    return false;
}

void ffParseMonitorJsonObject(FFMonitorOptions* options, yyjson_val* module)
{
    yyjson_val *key_, *val;
    size_t idx, max;
    yyjson_obj_foreach(module, idx, max, key_, val)
    {
        const char* key = yyjson_get_str(key_);
        if(ffStrEqualsIgnCase(key, "type"))
            continue;

        if (ffJsonConfigParseModuleArgs(key, val, &options->moduleArgs))
            continue;

        ffPrintError(FF_MONITOR_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "Unknown JSON key %s", key);
    }
}

void ffGenerateMonitorJsonConfig(FFMonitorOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    __attribute__((__cleanup__(ffDestroyMonitorOptions))) FFMonitorOptions defaultOptions;
    ffInitMonitorOptions(&defaultOptions);

    ffJsonConfigGenerateModuleArgsConfig(doc, module, &defaultOptions.moduleArgs, &options->moduleArgs);
}

void ffGenerateMonitorJsonResult(FF_MAYBE_UNUSED FFMonitorOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    FF_LIST_AUTO_DESTROY results = ffListCreate(sizeof(FFMonitorResult));

    const char* error = ffDetectMonitor(&results);

    if (error)
    {
        yyjson_mut_obj_add_str(doc, module, "error", error);
    }
    else if(results.length == 0)
    {
        yyjson_mut_obj_add_str(doc, module, "error", "No monitors found");
    }
    else
    {
        yyjson_mut_val* arr = yyjson_mut_obj_add_arr(doc, module, "result");
        FF_LIST_FOR_EACH(FFMonitorResult, item, results)
        {
            yyjson_mut_val* obj = yyjson_mut_arr_add_obj(doc, arr);
            yyjson_mut_obj_add_bool(doc, obj, "hdrCompatible", item->hdrCompatible);
            yyjson_mut_obj_add_strbuf(doc, obj, "name", &item->name);

            yyjson_mut_val* resolution = yyjson_mut_obj_add_obj(doc, obj, "resolution");
            yyjson_mut_obj_add_uint(doc, resolution, "width", item->width);
            yyjson_mut_obj_add_uint(doc, resolution, "height", item->height);

            yyjson_mut_val* physical = yyjson_mut_obj_add_obj(doc, obj, "physical");
            yyjson_mut_obj_add_uint(doc, physical, "height", item->physicalHeight);
            yyjson_mut_obj_add_uint(doc, physical, "width", item->physicalWidth);

            yyjson_mut_obj_add_real(doc, obj, "refreshRate", item->refreshRate);

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
        }
    }

    FF_LIST_FOR_EACH(FFMonitorResult, item, results)
    {
        ffStrbufDestroy(&item->name);
    }
}

void ffPrintMonitorHelpFormat(void)
{
    FF_PRINT_MODULE_FORMAT_HELP_CHECKED(FF_MONITOR_MODULE_NAME, "{2}x{3} px - {4}x{5} mm ({6} inches, {7} ppi)", FF_MONITOR_NUM_FORMAT_ARGS, ((const char* []) {
        "Display name - name",
        "Native resolution width in pixels - width",
        "Native resolution height in pixels - height",
        "Physical width in millimeters - physical-width",
        "Physical height in millimeters - physical-height",
        "Physical diagonal length in inches - inch",
        "Pixels per inch (PPI) - ppi",
        "Year of manufacturing - manufacture-year",
        "Nth week of manufacturing in the year - manufacture-week",
        "Serial number - serial",
        "Maximum refresh rate in Hz - refresh-rate",
    }));
}

void ffInitMonitorOptions(FFMonitorOptions* options)
{
    ffOptionInitModuleBaseInfo(
        &options->moduleInfo,
        FF_MONITOR_MODULE_NAME,
        "Print connected physical monitor information",
        ffParseMonitorCommandOptions,
        ffParseMonitorJsonObject,
        ffPrintMonitor,
        ffGenerateMonitorJsonResult,
        ffPrintMonitorHelpFormat,
        ffGenerateMonitorJsonConfig
    );
    ffOptionInitModuleArg(&options->moduleArgs, "󰹑");
}

void ffDestroyMonitorOptions(FFMonitorOptions* options)
{
    ffOptionDestroyModuleArg(&options->moduleArgs);
}
