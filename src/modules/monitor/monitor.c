#include "common/printing.h"
#include "common/jsonconfig.h"
#include "detection/monitor/monitor.h"
#include "modules/monitor/monitor.h"
#include "util/stringUtils.h"

#include <math.h>

#define FF_MONITOR_NUM_FORMAT_ARGS 7

void ffPrintMonitor(FFMonitorOptions* options)
{
    FF_LIST_AUTO_DESTROY result = ffListCreate(sizeof(FFMonitorResult));

    const char* error = ffDetectMonitor(&result);

    if(error)
    {
        ffPrintError(FF_MONITOR_MODULE_NAME, 0, &options->moduleArgs, "%s", error);
        return;
    }

    if(!result.length)
    {
        ffPrintError(FF_MONITOR_MODULE_NAME, 0, &options->moduleArgs, "No physical display detected");
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
            ffParseFormatString(&key, &options->moduleArgs.key, 2, (FFformatarg[]){
                {FF_FORMAT_ARG_TYPE_UINT, &moduleIndex},
                {FF_FORMAT_ARG_TYPE_STRBUF, &display->name},
            });
        }

        if(options->moduleArgs.outputFormat.length == 0)
        {
            ffPrintLogoAndKey(key.chars, 0, &options->moduleArgs, FF_PRINT_TYPE_NO_CUSTOM_KEY);

            printf("%ux%u px", display->width, display->height);
            if (inch > 0)
                printf(" - %ux%u mm (%.2f inches, %.2f ppi)\n", display->physicalWidth, display->physicalHeight, inch, ppi);
            else
                putchar('\n');
        }
        else
        {
            ffPrintFormatString(key.chars, 0, &options->moduleArgs, FF_PRINT_TYPE_NO_CUSTOM_KEY, FF_MONITOR_NUM_FORMAT_ARGS, (FFformatarg[]) {
                {FF_FORMAT_ARG_TYPE_STRBUF, &display->name},
                {FF_FORMAT_ARG_TYPE_UINT, &display->width},
                {FF_FORMAT_ARG_TYPE_UINT, &display->height},
                {FF_FORMAT_ARG_TYPE_UINT, &display->physicalWidth},
                {FF_FORMAT_ARG_TYPE_UINT, &display->physicalHeight},
                {FF_FORMAT_ARG_TYPE_DOUBLE, &inch},
                {FF_FORMAT_ARG_TYPE_DOUBLE, &ppi},
            });
        }

        ffStrbufDestroy(&display->name);
        ++index;
    }
}

void ffInitMonitorOptions(FFMonitorOptions* options)
{
    ffOptionInitModuleBaseInfo(&options->moduleInfo, FF_MONITOR_MODULE_NAME, ffParseMonitorCommandOptions, ffParseMonitorJsonObject, ffPrintMonitor, ffGenerateMonitorJson, ffPrintMonitorHelpFormat);
    ffOptionInitModuleArg(&options->moduleArgs);
}

bool ffParseMonitorCommandOptions(FFMonitorOptions* options, const char* key, const char* value)
{
    const char* subKey = ffOptionTestPrefix(key, FF_MONITOR_MODULE_NAME);
    if (!subKey) return false;
    if (ffOptionParseModuleArgs(key, subKey, value, &options->moduleArgs))
        return true;

    return false;
}

void ffDestroyMonitorOptions(FFMonitorOptions* options)
{
    ffOptionDestroyModuleArg(&options->moduleArgs);
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

        ffPrintError(FF_MONITOR_MODULE_NAME, 0, &options->moduleArgs, "Unknown JSON key %s", key);
    }
}

void ffGenerateMonitorJson(FF_MAYBE_UNUSED FFMonitorOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
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
            yyjson_mut_obj_add_uint(doc, obj, "width", item->width);
            yyjson_mut_obj_add_uint(doc, obj, "height", item->height);
            yyjson_mut_obj_add_strbuf(doc, obj, "name", &item->name);
            yyjson_mut_obj_add_uint(doc, obj, "physicalHeight", item->physicalHeight);
            yyjson_mut_obj_add_uint(doc, obj, "physicalWidth", item->physicalWidth);
        }
    }

    FF_LIST_FOR_EACH(FFMonitorResult, item, results)
    {
        ffStrbufDestroy(&item->name);
    }
}

void ffPrintMonitorHelpFormat(void)
{
    ffPrintModuleFormatHelp(FF_MONITOR_MODULE_NAME, "{2}x{3} px - {4}x{5} mm ({6} inches, {7} ppi)", FF_MONITOR_NUM_FORMAT_ARGS, (const char* []) {
        "Display name",
        "Display native resolution width in pixels",
        "Display native resolution height in pixels",
        "Display physical width in millimeters",
        "Display physical height in millimeters",
        "Display physical diagonal length in inches",
        "Display physical pixels per inch (PPI)"
    });
}
