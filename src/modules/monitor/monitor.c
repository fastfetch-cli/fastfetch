#include "common/printing.h"
#include "common/jsonconfig.h"
#include "detection/displayserver/displayserver.h"
#include "modules/monitor/monitor.h"
#include "util/stringUtils.h"

#include <math.h>

bool ffPrintMonitor(FFMonitorOptions* options)
{
    const FFDisplayServerResult* result = ffConnectDisplayServer();

    if(!result->displays.length)
    {
        ffPrintError(FF_MONITOR_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "No display detected");
        return false;
    }

    FF_STRBUF_AUTO_DESTROY key = ffStrbufCreate();
    uint32_t index = 0;
    FF_LIST_FOR_EACH(FFDisplayResult, display, result->displays)
    {
        double inch = sqrt(display->physicalWidth * display->physicalWidth + display->physicalHeight * display->physicalHeight) / 25.4;
        double ppi = sqrt(display->width * display->width + display->height * display->height) / inch;
        bool hdrCompatible = display->hdrStatus == FF_DISPLAY_HDR_STATUS_SUPPORTED || display->hdrStatus == FF_DISPLAY_HDR_STATUS_ENABLED;

        ffStrbufClear(&key);
        if(options->moduleArgs.key.length == 0)
        {
            ffStrbufAppendS(&key, FF_MONITOR_MODULE_NAME);
            if (display->name.length > 0)
                ffStrbufAppendF(&key, " (%s)", display->name.chars);
        }
        else
        {
            uint32_t moduleIndex = result->displays.length == 1 ? 0 : index + 1;
            FF_PARSE_FORMAT_STRING_CHECKED(&key, &options->moduleArgs.key, ((FFformatarg[]) {
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
                printf(" - %ux%u mm (%.2f inches, %.2f ppi)", display->physicalWidth, display->physicalHeight, inch, ppi);
            if (hdrCompatible)
                fputs(" [HDR Compatible]", stdout);
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

            FF_PRINT_FORMAT_CHECKED(key.chars, 0, &options->moduleArgs, FF_PRINT_TYPE_NO_CUSTOM_KEY, ((FFformatarg[]) {
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
                FF_FORMAT_ARG(hdrCompatible, "hdr-compatible"),
            }));
        }

        ffStrbufDestroy(&display->name);
        ++index;
    }

    return true;
}

void ffParseMonitorJsonObject(FFMonitorOptions* options, yyjson_val* module)
{
    yyjson_val *key, *val;
    size_t idx, max;
    yyjson_obj_foreach(module, idx, max, key, val)
    {
        if (ffJsonConfigParseModuleArgs(key, val, &options->moduleArgs))
            continue;

        ffPrintError(FF_MONITOR_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "Unknown JSON key %s", unsafe_yyjson_get_str(key));
    }
}

void ffGenerateMonitorJsonConfig(FFMonitorOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    ffJsonConfigGenerateModuleArgsConfig(doc, module, &options->moduleArgs);
}

bool ffGenerateMonitorJsonResult(FF_MAYBE_UNUSED FFMonitorOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    yyjson_mut_obj_add_str(doc, module, "error", "Monitor module is an alias of Display module");
    return false;
}

void ffInitMonitorOptions(FFMonitorOptions* options)
{
    ffOptionInitModuleArg(&options->moduleArgs, "󰹑");
}

void ffDestroyMonitorOptions(FFMonitorOptions* options)
{
    ffOptionDestroyModuleArg(&options->moduleArgs);
}

FFModuleBaseInfo ffMonitorModuleInfo = {
    .name = FF_MONITOR_MODULE_NAME,
    .description = "Alias of Display module",
    .initOptions = (void*) ffInitMonitorOptions,
    .destroyOptions = (void*) ffDestroyMonitorOptions,
    .parseJsonObject = (void*) ffParseMonitorJsonObject,
    .printModule = (void*) ffPrintMonitor,
    .generateJsonResult = (void*) ffGenerateMonitorJsonResult,
    .generateJsonConfig = (void*) ffGenerateMonitorJsonConfig,
    .formatArgs = FF_FORMAT_ARG_LIST(((FFModuleFormatArg[]) {
        {"Display name", "name"},
        {"Native resolution width in pixels", "width"},
        {"Native resolution height in pixels", "height"},
        {"Physical width in millimeters", "physical-width"},
        {"Physical height in millimeters", "physical-height"},
        {"Physical diagonal length in inches", "inch"},
        {"Pixels per inch (PPI)", "ppi"},
        {"Year of manufacturing", "manufacture-year"},
        {"Nth week of manufacturing in the year", "manufacture-week"},
        {"Serial number", "serial"},
        {"Maximum refresh rate in Hz", "refresh-rate"},
        {"True if the display is HDR compatible", "hdr-compatible"},
    }))
};
