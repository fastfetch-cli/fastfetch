#include "common/printing.h"
#include "common/jsonconfig.h"
#include "common/parsing.h"
#include "detection/netusage/netusage.h"
#include "modules/netusage/netusage.h"
#include "util/stringUtils.h"

#define FF_NETUSAGE_DISPLAY_NAME "Net Usage"
#define FF_NETUSAGE_NUM_FORMAT_ARGS 1

static int sortInfs(const FFNetUsageIoCounters* left, const FFNetUsageIoCounters* right)
{
    return ffStrbufComp(&left->name, &right->name);
}

static void formatKey(const FFNetUsageOptions* options, FFNetUsageIoCounters* inf, uint32_t index, FFstrbuf* key)
{
    if(options->moduleArgs.key.length == 0)
    {
        if(!inf->name.length)
            ffStrbufSetF(&inf->name, "unknown %u", (unsigned) index);

        ffStrbufSetF(key, FF_NETUSAGE_DISPLAY_NAME " (%s)", inf->name.chars);
    }
    else
    {
        ffStrbufClear(key);
        ffParseFormatString(key, &options->moduleArgs.key, 2, (FFformatarg[]){
            {FF_FORMAT_ARG_TYPE_UINT, &index},
            {FF_FORMAT_ARG_TYPE_STRBUF, &inf->name},
        });
    }
}

void ffPrintNetUsage(FFNetUsageOptions* options)
{
    FF_LIST_AUTO_DESTROY result = ffListCreate(sizeof(FFNetUsageIoCounters));
    const char* error = ffDetectNetUsage(&result);

    if(error)
    {
        ffPrintError(FF_NETUSAGE_DISPLAY_NAME, 0, &options->moduleArgs, "%s", error);
        return;
    }

    ffListSort(&result, (const void*) sortInfs);

    uint32_t index = 0;
    FF_STRBUF_AUTO_DESTROY key = ffStrbufCreate();
    FF_STRBUF_AUTO_DESTROY buffer = ffStrbufCreate();
    FF_STRBUF_AUTO_DESTROY buffer2 = ffStrbufCreate();

    FF_LIST_FOR_EACH(FFNetUsageIoCounters, inf, result)
    {
        formatKey(options, inf, result.length == 1 ? 0 : index + 1, &key);
        ffStrbufClear(&buffer);

        if(options->moduleArgs.outputFormat.length == 0)
        {
            ffPrintLogoAndKey(key.chars, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT);

            ffParseSize(inf->rxBytes, &buffer);
            ffStrbufAppendS(&buffer, "/s (in) - ");
            ffParseSize(inf->txBytes, &buffer);
            ffStrbufAppendS(&buffer, "/s (out)");
            ffStrbufPutTo(&buffer, stdout);
        }
        else
        {
            ffStrbufClear(&buffer2);
            ffParseSize(inf->rxBytes, &buffer);
            ffParseSize(inf->txBytes, &buffer2);
            ffPrintFormatString(key.chars, 0, &options->moduleArgs, FF_PRINT_TYPE_NO_CUSTOM_KEY, FF_NETUSAGE_NUM_FORMAT_ARGS, (FFformatarg[]){
                {FF_FORMAT_ARG_TYPE_STRBUF, &inf->name},
                {FF_FORMAT_ARG_TYPE_STRBUF, &buffer},
                {FF_FORMAT_ARG_TYPE_STRBUF, &buffer2},
                {FF_FORMAT_ARG_TYPE_UINT64, &inf->txBytes},
                {FF_FORMAT_ARG_TYPE_UINT64, &inf->rxBytes},
                {FF_FORMAT_ARG_TYPE_UINT64, &inf->txPackets},
                {FF_FORMAT_ARG_TYPE_UINT64, &inf->rxPackets},
                {FF_FORMAT_ARG_TYPE_UINT64, &inf->rxErrors},
                {FF_FORMAT_ARG_TYPE_UINT64, &inf->txErrors},
                {FF_FORMAT_ARG_TYPE_UINT64, &inf->rxDrops},
                {FF_FORMAT_ARG_TYPE_UINT64, &inf->txDrops},
            });
        }
        ++index;
    }
}

void ffInitNetUsageOptions(FFNetUsageOptions* options)
{
    ffOptionInitModuleBaseInfo(&options->moduleInfo, FF_NETUSAGE_MODULE_NAME, ffParseNetUsageCommandOptions, ffParseNetUsageJsonObject, ffPrintNetUsage, ffGenerateNetUsageJson);
    ffOptionInitModuleArg(&options->moduleArgs);
}

bool ffParseNetUsageCommandOptions(FFNetUsageOptions* options, const char* key, const char* value)
{
    const char* subKey = ffOptionTestPrefix(key, FF_NETUSAGE_MODULE_NAME);
    if (!subKey) return false;
    if (ffOptionParseModuleArgs(key, subKey, value, &options->moduleArgs))
        return true;

    return false;
}

void ffDestroyNetUsageOptions(FFNetUsageOptions* options)
{
    ffOptionDestroyModuleArg(&options->moduleArgs);
}

void ffParseNetUsageJsonObject(FFNetUsageOptions* options, yyjson_val* module)
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

        ffPrintError(FF_NETUSAGE_MODULE_NAME, 0, &options->moduleArgs, "Unknown JSON key %s", key);
    }
}

void ffGenerateNetUsageJson(FF_MAYBE_UNUSED FFNetUsageOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    FF_LIST_AUTO_DESTROY result = ffListCreate(sizeof(FFNetUsageIoCounters));
    const char* error = ffDetectNetUsage(&result);

    if(error)
    {
        yyjson_mut_obj_add_str(doc, module, "error", error);
        return;
    }

    yyjson_mut_val* arr = yyjson_mut_obj_add_arr(doc, module, "result");
    FF_LIST_FOR_EACH(FFNetUsageIoCounters, counter, result)
    {
        yyjson_mut_val* obj = yyjson_mut_arr_add_obj(doc, arr);
        yyjson_mut_obj_add_strbuf(doc, obj, "name", &counter->name);
        yyjson_mut_obj_add_uint(doc, obj, "txBytes", counter->txBytes);
        yyjson_mut_obj_add_uint(doc, obj, "rxBytes", counter->rxBytes);
        yyjson_mut_obj_add_uint(doc, obj, "txPackets", counter->txPackets);
        yyjson_mut_obj_add_uint(doc, obj, "rxPackets", counter->rxPackets);
        yyjson_mut_obj_add_uint(doc, obj, "rxErrors", counter->rxErrors);
        yyjson_mut_obj_add_uint(doc, obj, "txErrors", counter->txErrors);
        yyjson_mut_obj_add_uint(doc, obj, "rxDrops", counter->rxDrops);
        yyjson_mut_obj_add_uint(doc, obj, "txDrops", counter->txDrops);

    }
}
