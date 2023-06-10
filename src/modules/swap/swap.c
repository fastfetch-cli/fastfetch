#include "common/printing.h"
#include "common/jsonconfig.h"
#include "common/parsing.h"
#include "common/bar.h"
#include "detection/swap/swap.h"
#include "modules/swap/swap.h"

#define FF_SWAP_NUM_FORMAT_ARGS 3

void ffPrintSwap(FFinstance* instance, FFSwapOptions* options)
{
    FFSwapResult storage;
    const char* error = ffDetectSwap(&storage);

    if(error)
    {
        ffPrintError(instance, FF_SWAP_MODULE_NAME, 0, &options->moduleArgs, "%s", error);
        return;
    }

    FF_STRBUF_AUTO_DESTROY usedPretty = ffStrbufCreate();
    ffParseSize(storage.bytesUsed, instance->config.binaryPrefixType, &usedPretty);

    FF_STRBUF_AUTO_DESTROY totalPretty = ffStrbufCreate();
    ffParseSize(storage.bytesTotal, instance->config.binaryPrefixType, &totalPretty);

    uint8_t percentage = storage.bytesTotal == 0
        ? 0
        : (uint8_t) (((long double) storage.bytesUsed / (long double) storage.bytesTotal) * 100.0);

    if(options->moduleArgs.outputFormat.length == 0)
    {
        ffPrintLogoAndKey(instance, FF_SWAP_MODULE_NAME, 0, &options->moduleArgs.key);
        if (storage.bytesTotal == 0)
            puts("Disabled");
        else
        {
            FF_STRBUF_AUTO_DESTROY str = ffStrbufCreate();

            if(instance->config.percentType & FF_PERCENTAGE_TYPE_BAR_BIT)
            {
                ffAppendPercentBar(instance, &str, percentage, 0, 5, 8);
                ffStrbufAppendC(&str, ' ');
            }

            if(!(instance->config.percentType & FF_PERCENTAGE_TYPE_HIDE_OTHERS_BIT))
                ffStrbufAppendF(&str, "%s / %s ", usedPretty.chars, totalPretty.chars);

            if(instance->config.percentType & FF_PERCENTAGE_TYPE_NUM_BIT)
                ffAppendPercentNum(instance, &str, (uint8_t) percentage, 50, 80, str.length > 0);

            ffStrbufTrimRight(&str, ' ');
            ffStrbufPutTo(&str, stdout);
        }
    }
    else
    {
        ffPrintFormat(instance, FF_SWAP_MODULE_NAME, 0, &options->moduleArgs, FF_SWAP_NUM_FORMAT_ARGS, (FFformatarg[]){
            {FF_FORMAT_ARG_TYPE_STRBUF, &usedPretty},
            {FF_FORMAT_ARG_TYPE_STRBUF, &totalPretty},
            {FF_FORMAT_ARG_TYPE_UINT8, &percentage},
        });
    }
}

void ffInitSwapOptions(FFSwapOptions* options)
{
    options->moduleName = FF_SWAP_MODULE_NAME;
    ffOptionInitModuleArg(&options->moduleArgs);
}

bool ffParseSwapCommandOptions(FFSwapOptions* options, const char* key, const char* value)
{
    const char* subKey = ffOptionTestPrefix(key, FF_SWAP_MODULE_NAME);
    if (!subKey) return false;
    if (ffOptionParseModuleArgs(key, subKey, value, &options->moduleArgs))
        return true;

    return false;
}

void ffDestroySwapOptions(FFSwapOptions* options)
{
    ffOptionDestroyModuleArg(&options->moduleArgs);
}

void ffParseSwapJsonObject(FFinstance* instance, yyjson_val* module)
{
    FFSwapOptions __attribute__((__cleanup__(ffDestroySwapOptions))) options;
    ffInitSwapOptions(&options);

    if (module)
    {
        yyjson_val *key_, *val;
        size_t idx, max;
        yyjson_obj_foreach(module, idx, max, key_, val)
        {
            const char* key = yyjson_get_str(key_);
            if(strcasecmp(key, "type") == 0)
                continue;

            if (ffJsonConfigParseModuleArgs(key, val, &options.moduleArgs))
                continue;

            ffPrintError(instance, FF_SWAP_MODULE_NAME, 0, &options.moduleArgs, "Unknown JSON key %s", key);
        }
    }

    ffPrintSwap(instance, &options);
}
