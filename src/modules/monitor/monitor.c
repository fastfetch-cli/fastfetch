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
    ffOptionInitModuleBaseInfo(&options->moduleInfo, FF_MONITOR_MODULE_NAME, ffParseMonitorCommandOptions, ffParseMonitorJsonObject, ffPrintMonitor);
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
