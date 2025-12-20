#include "common/printing.h"
#include "common/jsonconfig.h"
#include "common/size.h"
#include "detection/netio/netio.h"
#include "modules/netio/netio.h"
#include "util/stringUtils.h"

#define FF_NETIO_DISPLAY_NAME "Network IO"

static int sortInfs(const FFNetIOResult* left, const FFNetIOResult* right)
{
    return ffStrbufComp(&left->name, &right->name);
}

static void formatKey(const FFNetIOOptions* options, FFNetIOResult* inf, uint32_t index, FFstrbuf* key)
{
    if(options->moduleArgs.key.length == 0)
    {
        if(!inf->name.length)
            ffStrbufSetF(&inf->name, "unknown %u", (unsigned) index);

        ffStrbufSetF(key, FF_NETIO_DISPLAY_NAME " (%s)", inf->name.chars);
    }
    else
    {
        ffStrbufClear(key);
        FF_PARSE_FORMAT_STRING_CHECKED(key, &options->moduleArgs.key, ((FFformatarg[]){
            FF_FORMAT_ARG(index, "index"),
            FF_FORMAT_ARG(inf->name, "name"),
            FF_FORMAT_ARG(options->moduleArgs.keyIcon, "icon"),
        }));
    }
}

bool ffPrintNetIO(FFNetIOOptions* options)
{
    FF_LIST_AUTO_DESTROY result = ffListCreate(sizeof(FFNetIOResult));
    const char* error = ffDetectNetIO(&result, options);

    if(error)
    {
        ffPrintError(FF_NETIO_DISPLAY_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "%s", error);
        return false;
    }

    ffListSort(&result, (const void*) sortInfs);

    uint32_t index = 0;
    FF_STRBUF_AUTO_DESTROY key = ffStrbufCreate();
    FF_STRBUF_AUTO_DESTROY buffer = ffStrbufCreate();
    FF_STRBUF_AUTO_DESTROY buffer2 = ffStrbufCreate();

    FF_LIST_FOR_EACH(FFNetIOResult, inf, result)
    {
        formatKey(options, inf, result.length == 1 ? 0 : index + 1, &key);
        ffStrbufClear(&buffer);

        if(options->moduleArgs.outputFormat.length == 0)
        {
            ffPrintLogoAndKey(key.chars, 0, &options->moduleArgs, FF_PRINT_TYPE_NO_CUSTOM_KEY);

            ffSizeAppendNum(inf->rxBytes, &buffer);
            if (!options->detectTotal) ffStrbufAppendS(&buffer, "/s");
            ffStrbufAppendS(&buffer, " (IN) - ");

            ffSizeAppendNum(inf->txBytes, &buffer);
            if (!options->detectTotal) ffStrbufAppendS(&buffer, "/s");
            ffStrbufAppendS(&buffer, " (OUT)");

            if (inf->defaultRoute && !options->defaultRouteOnly)
                ffStrbufAppendS(&buffer, " *");
            ffStrbufPutTo(&buffer, stdout);
        }
        else
        {
            ffStrbufClear(&buffer2);
            ffSizeAppendNum(inf->rxBytes, &buffer);
            if (!options->detectTotal) ffStrbufAppendS(&buffer, "/s");
            ffSizeAppendNum(inf->txBytes, &buffer2);
            if (!options->detectTotal) ffStrbufAppendS(&buffer2, "/s");

            FF_PRINT_FORMAT_CHECKED(key.chars, 0, &options->moduleArgs, FF_PRINT_TYPE_NO_CUSTOM_KEY, ((FFformatarg[]){
                FF_FORMAT_ARG(buffer, "rx-size"),
                FF_FORMAT_ARG(buffer2, "tx-size"),
                FF_FORMAT_ARG(inf->name, "ifname"),
                FF_FORMAT_ARG(inf->defaultRoute, "is-default-route"),
                FF_FORMAT_ARG(inf->txBytes, "tx-bytes"),
                FF_FORMAT_ARG(inf->rxBytes, "rx-bytes"),
                FF_FORMAT_ARG(inf->txPackets, "tx-packets"),
                FF_FORMAT_ARG(inf->rxPackets, "rx-packets"),
                FF_FORMAT_ARG(inf->rxErrors, "rx-errors"),
                FF_FORMAT_ARG(inf->txErrors, "tx-errors"),
                FF_FORMAT_ARG(inf->rxDrops, "rx-drops"),
                FF_FORMAT_ARG(inf->txDrops, "tx-drops"),
            }));
        }
        ++index;
    }

    FF_LIST_FOR_EACH(FFNetIOResult, inf, result)
    {
        ffStrbufDestroy(&inf->name);
    }

    return true;
}

void ffParseNetIOJsonObject(FFNetIOOptions* options, yyjson_val* module)
{
    yyjson_val *key, *val;
    size_t idx, max;
    yyjson_obj_foreach(module, idx, max, key, val)
    {
        if (ffJsonConfigParseModuleArgs(key, val, &options->moduleArgs))
            continue;

        if (unsafe_yyjson_equals_str(key, "namePrefix"))
        {
            ffStrbufSetJsonVal(&options->namePrefix, val);
            continue;
        }

        if (unsafe_yyjson_equals_str(key, "defaultRouteOnly"))
        {
            options->defaultRouteOnly = yyjson_get_bool(val);
            continue;
        }

        if (unsafe_yyjson_equals_str(key, "detectTotal"))
        {
            options->detectTotal = yyjson_get_bool(val);
            continue;
        }

        if (unsafe_yyjson_equals_str(key, "waitTime"))
        {
            options->waitTime = (uint32_t) yyjson_get_uint(val);
            continue;
        }

        ffPrintError(FF_NETIO_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "Unknown JSON key %s", unsafe_yyjson_get_str(key));
    }
}

void ffGenerateNetIOJsonConfig(FFNetIOOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    ffJsonConfigGenerateModuleArgsConfig(doc, module, &options->moduleArgs);

    yyjson_mut_obj_add_strbuf(doc, module, "namePrefix", &options->namePrefix);

    yyjson_mut_obj_add_bool(doc, module, "defaultRouteOnly", options->defaultRouteOnly);

    yyjson_mut_obj_add_bool(doc, module, "detectTotal", options->detectTotal);

    yyjson_mut_obj_add_uint(doc, module, "waitTime", options->waitTime);
}

bool ffGenerateNetIOJsonResult(FFNetIOOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    FF_LIST_AUTO_DESTROY result = ffListCreate(sizeof(FFNetIOResult));
    const char* error = ffDetectNetIO(&result, options);

    if(error)
    {
        yyjson_mut_obj_add_str(doc, module, "error", error);
        return false;
    }

    yyjson_mut_val* arr = yyjson_mut_obj_add_arr(doc, module, "result");
    FF_LIST_FOR_EACH(FFNetIOResult, counter, result)
    {
        yyjson_mut_val* obj = yyjson_mut_arr_add_obj(doc, arr);
        yyjson_mut_obj_add_strbuf(doc, obj, "name", &counter->name);
        yyjson_mut_obj_add_bool(doc, obj, "defaultRoute", counter->defaultRoute);
        yyjson_mut_obj_add_uint(doc, obj, "txBytes", counter->txBytes);
        yyjson_mut_obj_add_uint(doc, obj, "rxBytes", counter->rxBytes);
        yyjson_mut_obj_add_uint(doc, obj, "txPackets", counter->txPackets);
        yyjson_mut_obj_add_uint(doc, obj, "rxPackets", counter->rxPackets);
        yyjson_mut_obj_add_uint(doc, obj, "rxErrors", counter->rxErrors);
        yyjson_mut_obj_add_uint(doc, obj, "txErrors", counter->txErrors);
        yyjson_mut_obj_add_uint(doc, obj, "rxDrops", counter->rxDrops);
        yyjson_mut_obj_add_uint(doc, obj, "txDrops", counter->txDrops);
    }

    FF_LIST_FOR_EACH(FFNetIOResult, inf, result)
    {
        ffStrbufDestroy(&inf->name);
    }

    return true;
}

void ffInitNetIOOptions(FFNetIOOptions* options)
{
    ffOptionInitModuleArg(&options->moduleArgs, "󰾆");

    ffStrbufInit(&options->namePrefix);
    options->defaultRouteOnly =
        #if __ANDROID__
            false
        #else
            true
        #endif
    ;
    options->detectTotal = false;
    options->waitTime = 1000;
}

void ffDestroyNetIOOptions(FFNetIOOptions* options)
{
    ffOptionDestroyModuleArg(&options->moduleArgs);
    ffStrbufDestroy(&options->namePrefix);
}

FFModuleBaseInfo ffNetIOModuleInfo = {
    .name = FF_NETIO_MODULE_NAME,
    .description = "Print network I/O throughput",
    .initOptions = (void*) ffInitNetIOOptions,
    .destroyOptions = (void*) ffDestroyNetIOOptions,
    .parseJsonObject = (void*) ffParseNetIOJsonObject,
    .printModule = (void*) ffPrintNetIO,
    .generateJsonResult = (void*) ffGenerateNetIOJsonResult,
    .generateJsonConfig = (void*) ffGenerateNetIOJsonConfig,
    .formatArgs = FF_FORMAT_ARG_LIST(((FFModuleFormatArg[]) {
        {"Size of data received [per second] (formatted)", "rx-size"},
        {"Size of data sent [per second] (formatted)", "tx-size"},
        {"Interface name", "ifname"},
        {"Is default route", "is-default-route"},
        {"Size of data received [per second] (in bytes)", "rx-bytes"},
        {"Size of data sent [per second] (in bytes)", "tx-bytes"},
        {"Number of packets received [per second]", "rx-packets"},
        {"Number of packets sent [per second]", "tx-packets"},
        {"Number of errors received [per second]", "rx-errors"},
        {"Number of errors sent [per second]", "tx-errors"},
        {"Number of packets dropped when receiving [per second]", "rx-drops"},
        {"Number of packets dropped when sending [per second]", "tx-drops"},
    }))
};
